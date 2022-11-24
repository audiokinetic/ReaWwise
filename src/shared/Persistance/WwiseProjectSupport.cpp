/*----------------------------------------------------------------------------------------

Copyright (c) 2023 AUDIOKINETIC Inc.

This file is licensed to use under the license available at:
https://github.com/audiokinetic/ReaWwise/blob/main/License.txt (the "License").
You may not use this file except in compliance with the License.

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.  See the License for the
specific language governing permissions and limitations under the License.

----------------------------------------------------------------------------------------*/

#include "WwiseProjectSupport.h"

#include "Helpers/ImportHelper.h"
#include "Model/IDs.h"

namespace AK::WwiseTransfer
{
	WwiseProjectSupport::WwiseProjectSupport(juce::ValueTree appState, WaapiClient& waapiClient)
		: applicationState(appState)
		, waapiClient(waapiClient)
		, waapiConnected(applicationState, IDs::waapiConnected, nullptr)
		, projectPath(applicationState, IDs::projectPath, nullptr)
		, projectId(applicationState, IDs::projectId, nullptr)
		, originalsFolder(applicationState, IDs::originalsFolder, nullptr)
		, languageSubfolder(applicationState, IDs::languageSubfolder, nullptr)
		, referenceLanguage(applicationState, IDs::referenceLanguage, nullptr)
		, hierarchyMapping(applicationState.getChildWithName(IDs::hierarchyMapping))
		, languages(applicationState.getChildWithName(IDs::languages))
	{
		auto featureSupport = applicationState.getChildWithName(IDs::featureSupport);
		additionalProjectInfoLookupEnabled.referTo(featureSupport, IDs::additionalProjectInfoLookupEnabled, nullptr);

		applicationState.addListener(this);
	}

	WwiseProjectSupport::~WwiseProjectSupport()
	{
		applicationState.removeListener(this);
	}

	void WwiseProjectSupport::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
	{
		auto treeType = treeWhosePropertyHasChanged.getType();

		if(treeType == IDs::application)
		{
			if(property == IDs::additionalProjectInfoLookupEnabled)
			{
				additionalProjectInfoLookupEnabled.forceUpdateOfCachedValue();

				if(additionalProjectInfoLookupEnabled.get())
					loadAdditionalProjectInfo();
				else
					resetAdditionalProjectInfo();
			}
			else if(property == IDs::projectId)
			{
				projectId.forceUpdateOfCachedValue();

				// Project id is set to empty by the waapi client when it receives the project loaded event.
				// This means that a project was loaded in wwise and that we should refresh the project info
				if(projectId.get().isEmpty())
					loadProjectInfo();
				// If the project id changed, and is not empty, this means that the basic project info was loaded successfully.
				// Proceed to load additional data
				else
				{
					// Available in all supported versions of wwise
					loadProjectLanguages();

					// Extra information only available in wwise 2022+
					if(additionalProjectInfoLookupEnabled.get())
						loadAdditionalProjectInfo();
				}
			}
			else if(property == IDs::waapiConnected)
			{
				waapiConnected.forceUpdateOfCachedValue();

				if(waapiConnected.get())
					loadProjectInfo();
			}
		}
		else if(treeType == IDs::hierarchyMappingNode)
		{
			// Update hierarchy mapping node language depending on type
			if(property == IDs::objectType)
				updateLangugeForHierarchyMappingNodes();

			// Will update the language subpath if the last item in the hierarchy mapping is a SoundVoice and it's value has changed
			else if(property == IDs::objectLanguage)
				updateLanguageSubpath();
		}
	}

	void WwiseProjectSupport::valueTreeChildAdded(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded)
	{
		juce::ignoreUnused(childWhichHasBeenAdded);

		// Ensures that a newly added sound voice node gets updated with the propper language
		if(parentTree.getType() == IDs::hierarchyMapping ||
			// If the languages in wwise change, or a new project is loaded with different languages,
		    // we want to make sure sound voice nodes in the hierarchy mapping are updated.
			parentTree.getType() == IDs::languages)
		{
			updateLangugeForHierarchyMappingNodes();
		}
	}

	void WwiseProjectSupport::valueTreeChildRemoved(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved)
	{
		juce::ignoreUnused(childWhichHasBeenRemoved, indexFromWhichChildWasRemoved);

		// Language subpath can change depending on what hierarchy mapping nodes are present
		if(parentTree.getType() == IDs::hierarchyMapping)
		{
			updateLanguageSubpath();
		}
	}

	void WwiseProjectSupport::valueTreeChildOrderChanged(juce::ValueTree& parentTreeWhoseChildrenHaveMoved, int oldIndex, int newIndex)
	{
		juce::ignoreUnused(oldIndex, newIndex);

		// Language subpath may change depending on the order of the hierarchy mapping nodes
		if(parentTreeWhoseChildrenHaveMoved.getType() == IDs::hierarchyMapping)
		{
			updateLanguageSubpath();
		}
	}

	void WwiseProjectSupport::loadProjectInfo()
	{
		auto onGetProjectInfoAsync = [this](const auto& response)
		{
			projectPath = response.result.projectPath;
			projectId = response.result.projectId;
		};

		waapiClient.getProjectInfoAsync(onGetProjectInfoAsync);
	}

	void WwiseProjectSupport::loadAdditionalProjectInfo()
	{
		auto onGetAdditionalProjectInfoAsync = [this](const auto& response)
		{
			originalsFolder = response.result.originalsFolder;
			referenceLanguage = response.result.referenceLanguage;
		};

		waapiClient.getAdditionalProjectInfoAsync(onGetAdditionalProjectInfoAsync);
	}

	void WwiseProjectSupport::resetAdditionalProjectInfo()
	{
		originalsFolder = "";
		referenceLanguage = "";
	}

	void WwiseProjectSupport::loadProjectLanguages()
	{
		auto onGetProjectLanguages = [this](const auto& response)
		{
			if(response.status)
			{
				languages.copyPropertiesAndChildrenFrom(WwiseHelper::languagesToValueTree(response.result), nullptr);
			}
		};

		waapiClient.getProjectLanguagesAsync(onGetProjectLanguages);
	}

	void WwiseProjectSupport::updateLanguageSubpath()
	{
		juce::String subfolder("SFX");

		auto hierarchyMappingNodeList = ImportHelper::valueTreeToHierarchyMappingNodeList(hierarchyMapping);

		if(!hierarchyMappingNodeList.empty())
		{
			auto lastNode = hierarchyMappingNodeList.back();

			if(lastNode.type == Wwise::ObjectType::SoundVoice && lastNode.language.isNotEmpty())
			{
				subfolder = juce::String("Voices") + juce::File::getSeparatorChar() + lastNode.language;
			}
		}

		languageSubfolder = subfolder;
	}

	void WwiseProjectSupport::updateLangugeForHierarchyMappingNodes()
	{
		referenceLanguage.forceUpdateOfCachedValue();

		auto languageList = WwiseHelper::valueTreeToLanguages(languages);

		for(int i = 0; i < hierarchyMapping.getNumChildren(); ++i)
		{
			auto hierarchyMappingNode = hierarchyMapping.getChild(i);
			const Wwise::ObjectType objectType = juce::VariantConverter<Wwise::ObjectType>::fromVar(hierarchyMappingNode[IDs::objectType]);

			if(objectType == Wwise::ObjectType::SoundVoice)
			{
				juce::String objectLanguage = hierarchyMappingNode[IDs::objectLanguage];

				auto it = std::find_if(languageList.cbegin(), languageList.cend(), [&objectLanguage](const auto& referenceLanguage)
					{
						return objectLanguage == referenceLanguage;
					});

				if(it == languageList.cend())
				{
					if(referenceLanguage.get().isNotEmpty())
						hierarchyMappingNode.setProperty(IDs::objectLanguage, referenceLanguage.get(), nullptr);
					else if(!languageList.empty())
						hierarchyMappingNode.setProperty(IDs::objectLanguage, languageList[0], nullptr);
				}
			}
			else
			{
				hierarchyMappingNode.setProperty(IDs::objectLanguage, juce::String(), nullptr);
			}
		}
	}
} // namespace AK::WwiseTransfer
