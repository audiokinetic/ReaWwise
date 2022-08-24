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
		, hierarchyMapping(applicationState.getChildWithName(IDs::hierarchyMapping))
		, languages(applicationState.getChildWithName(IDs::languages))
	{
		auto featureSupport = applicationState.getChildWithName(IDs::featureSupport);
		originalsFolderLookupEnabled.referTo(featureSupport, IDs::originalsFolderLookupEnabled, nullptr);

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
			if(property == IDs::originalsFolderLookupEnabled)
			{
				originalsFolderLookupEnabled.forceUpdateOfCachedValue();
				if(originalsFolderLookupEnabled.get())
				{
					loadOriginalsFolder();
				}
				else
				{
					originalsFolder = "";
				}
			}
			else if(property == IDs::projectId)
			{
				// Project id is set to empty by the waapi client when it receives the project loaded event.
				// This means that a project was loaded in wwise and that we should refresh the project info
				projectId.forceUpdateOfCachedValue();
				if(projectId.get().isEmpty())
				{
					loadProjectInfo();
				}
				else
				{
					loadProjectLanguages();

					if(originalsFolderLookupEnabled.get())
					{
						loadOriginalsFolder();
					}
				}
			}
			else if(property == IDs::waapiConnected)
			{
				waapiConnected.forceUpdateOfCachedValue();

				if(waapiConnected.get())
				{
					loadProjectInfo();
				}
				else
				{
					projectPath = "";
					projectId = "";
					originalsFolder = "";
				}
			}
		}
		else if(treeType == IDs::hierarchyMapping ||
				treeType == IDs::hierarchyMappingNode)
		{
			updateLanguageSubpath();
		}
	}

	void WwiseProjectSupport::valueTreeChildAdded(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded)
	{
		juce::ignoreUnused(childWhichHasBeenAdded);

		if(parentTree.getType() == IDs::hierarchyMapping)
		{
			updateLanguageSubpath();
		}
	}

	void WwiseProjectSupport::valueTreeChildRemoved(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved)
	{
		juce::ignoreUnused(childWhichHasBeenRemoved, indexFromWhichChildWasRemoved);

		if(parentTree.getType() == IDs::hierarchyMapping)
		{
			updateLanguageSubpath();
		}
	}

	void WwiseProjectSupport::valueTreeChildOrderChanged(juce::ValueTree& parentTreeWhoseChildrenHaveMoved, int oldIndex, int newIndex)
	{
		juce::ignoreUnused(oldIndex, newIndex);

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

	void WwiseProjectSupport::loadOriginalsFolder()
	{
		auto onGetOriginalsFolderAsync = [this](const auto& response)
		{
			originalsFolder = response.result;
		};

		waapiClient.getOriginalsFolderAsync(onGetOriginalsFolderAsync);
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
} // namespace AK::WwiseTransfer
