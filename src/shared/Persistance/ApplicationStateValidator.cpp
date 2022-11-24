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

#include "ApplicationStateValidator.h"

#include "Helpers/ImportHelper.h"
#include "Helpers/WwiseHelper.h"
#include "Model/IDs.h"

namespace AK::WwiseTransfer::ApplicationState
{
	Validator::Validator(juce::ValueTree appState, WaapiClient& waapiClient)
		: applicationState(appState)
		, waapiClient(waapiClient)
	{
		applicationState.addListener(this);
	}

	Validator::~Validator()
	{
		applicationState.removeListener(this);
	}

	void Validator::valueTreePropertyChanged(juce::ValueTree& valueTree, const juce::Identifier& property)
	{
		if(valueTree.getType() == IDs::application)
		{
			if(property == IDs::originalsSubfolder || property == IDs::languageSubfolder)
			{
				const juce::String originalsFolder = valueTree[IDs::originalsFolder];
				const juce::String originalsSubfolder = valueTree[IDs::originalsSubfolder];
				const juce::String languageSubfolder = valueTree[IDs::languageSubfolder];

				auto isValid = validateOriginalsSubfolder(originalsFolder, languageSubfolder, originalsSubfolder);
				juce::String errorMessage = isValid ? "" : "Invalid originals subfolder";

				valueTree.setPropertyExcludingListener(this, IDs::originalsSubfolderValid, isValid, nullptr);
				valueTree.setPropertyExcludingListener(this, IDs::originalsSubfolderErrorMessage, errorMessage, nullptr);
			}
			else if(property == IDs::importDestination)
			{
				const juce::String importDestination = valueTree[IDs::importDestination];

				auto isValid = validateImportDestination(importDestination);
				juce::String errorMessage = isValid ? "" : "Invalid import destination";

				valueTree.setPropertyExcludingListener(this, IDs::importDestinationValid, isValid, nullptr);
				valueTree.setPropertyExcludingListener(this, IDs::importDestinationErrorMessage, errorMessage, nullptr);

				auto onGetObjectAsync = [this](const Waapi::Response<Waapi::ObjectResponse> response)
				{
					auto objectType = Wwise::ObjectType::VirtualFolder;

					if(response.result.path.isNotEmpty())
						objectType = response.result.type;

					applicationState.setProperty(IDs::importDestinationType, juce::VariantConverter<Wwise::ObjectType>::toVar(objectType), nullptr);
				};

				waapiClient.getObjectAsync(importDestination, onGetObjectAsync);
			}
			else if(property == IDs::importDestinationType)
			{
				const auto importDestinationType = juce::VariantConverter<Wwise::ObjectType>::fromVar(valueTree[IDs::importDestinationType]);

				validateHierarchyMapping(importDestinationType, applicationState.getChildWithName(IDs::hierarchyMapping));
			}
		}
		else if(valueTree.getType() == IDs::hierarchyMappingNode)
		{
			if(property == IDs::objectType)
			{
				auto hierarchyMapping = valueTree.getParent();

				const auto importDestinationType = juce::VariantConverter<Wwise::ObjectType>::fromVar(applicationState[IDs::importDestinationType]);
				validateHierarchyMapping(importDestinationType, hierarchyMapping);
			}
			else if(property == IDs::propertyTemplatePath || property == IDs::propertyTemplatePathType)
			{
				validatePropertyTemplatePath(valueTree);
			}
			else if(property == IDs::objectName)
			{
				validateObjectName(valueTree);
			}
		}
	}

	void Validator::valueTreeChildAdded(juce::ValueTree& parent, juce::ValueTree& child)
	{
		if(parent.getType() == IDs::hierarchyMapping)
		{
			const auto importDestinationType = juce::VariantConverter<Wwise::ObjectType>::fromVar(applicationState[IDs::importDestinationType]);

			validateHierarchyMapping(importDestinationType, parent);
		}

		if(child.getType() == IDs::hierarchyMappingNode)
		{
			validatePropertyTemplatePath(child);
			validateObjectName(child);
		}
	}

	void Validator::valueTreeChildRemoved(juce::ValueTree& parent, juce::ValueTree& child, int indexOfChild)
	{
		if(parent.getType() == IDs::hierarchyMapping)
		{
			const auto importDestinationType = juce::VariantConverter<Wwise::ObjectType>::fromVar(applicationState[IDs::importDestinationType]);

			validateHierarchyMapping(importDestinationType, parent);
		}
	}

	void Validator::valueTreeChildOrderChanged(juce::ValueTree& parent, int oldIndex, int newIndex)
	{
		if(parent.getType() == IDs::hierarchyMapping)
		{
			const auto importDestinationType = juce::VariantConverter<Wwise::ObjectType>::fromVar(applicationState[IDs::importDestinationType]);

			validateHierarchyMapping(importDestinationType, parent);
		}
	}

	bool Validator::validateOriginalsSubfolder(const juce::String& originalsFolder, const juce::String& languageSubfolder, const juce::String& originalsSubfolder)
	{
		if(originalsFolder.isEmpty() || originalsSubfolder.isEmpty())
			return true;

		auto originalsFolderWithLanguageSubfolder = juce::File(originalsFolder).getChildFile(languageSubfolder);

		auto originalsSubfolderAbsolutePath = originalsFolderWithLanguageSubfolder.getChildFile(originalsSubfolder);

		return originalsSubfolderAbsolutePath.isAChildOf(originalsFolderWithLanguageSubfolder);
	}

	bool Validator::validateImportDestination(const juce::String& importDestination)
	{
		using namespace Wwise;

		static const juce::String pathPrefix = "\\Actor-Mixer Hierarchy";

		auto allowedPathPrefix = importDestination.startsWith(pathPrefix);

		return importDestination.isNotEmpty() && !importDestination.endsWith("\\") && allowedPathPrefix;
	}

	void Validator::validatePropertyTemplatePath(juce::ValueTree hierarchyMappingNode)
	{
		using namespace Wwise;

		const juce::String propertyTemplatePath = hierarchyMappingNode[IDs::propertyTemplatePath];
		const auto propertyTemplatePathType = juce::VariantConverter<Wwise::ObjectType>::fromVar(hierarchyMappingNode[IDs::propertyTemplatePathType]);

		static const std::initializer_list<ObjectType> allowedObjectTypes = {ObjectType::WorkUnit,
			ObjectType::RandomContainer, ObjectType::BlendContainer, ObjectType::ActorMixer, ObjectType::SwitchContainer,
			ObjectType::Sound};

		static const juce::String pathPrefix = "\\Actor-Mixer Hierarchy";

		auto allowedType = std::find(allowedObjectTypes.begin(), allowedObjectTypes.end(),
							   propertyTemplatePathType) != allowedObjectTypes.end() ||
		                   propertyTemplatePathType == ObjectType::Unknown;

		auto allowedPathPrefix = propertyTemplatePath.startsWith(pathPrefix);

		bool isValid = propertyTemplatePath.isEmpty() || allowedType && allowedPathPrefix;
		juce::String errorMessage = isValid ? "" : "Invalid property template path";

		hierarchyMappingNode.setPropertyExcludingListener(this, IDs::propertyTemplatePathValid, isValid, nullptr);
		hierarchyMappingNode.setPropertyExcludingListener(this, IDs::propertyTemplatePathErrorMessage, errorMessage, nullptr);
	}

	void Validator::validateObjectName(juce::ValueTree hierarchyMappingNode)
	{
		juce::String objectName = hierarchyMappingNode[IDs::objectName];

		bool isValid = objectName.isNotEmpty();
		juce::String errorMessage = isValid ? "" : "Object name cannot be empty";

		hierarchyMappingNode.setPropertyExcludingListener(this, IDs::objectNameValid, isValid, nullptr);
		hierarchyMappingNode.setPropertyExcludingListener(this, IDs::objectNameErrorMessage, errorMessage, nullptr);
	}

	void Validator::validateHierarchyMapping(Wwise::ObjectType importDestinationType, juce::ValueTree hierarchyMapping)
	{
		// To properly validate the hierarchy mapping, we must include the import destination
		std::vector<Wwise::ObjectType> hierarchyTypes{importDestinationType};
		for(int i = 0; i < hierarchyMapping.getNumChildren(); ++i)
		{
			const auto hierarchyMappingNode = hierarchyMapping.getChild(i);
			hierarchyTypes.emplace_back(juce::VariantConverter<Wwise::ObjectType>::fromVar(hierarchyMappingNode[IDs::objectType]));
		}

		for(std::size_t i = 1; i < hierarchyTypes.size(); ++i)
		{
			const auto child = hierarchyTypes[i];
			const auto parent = hierarchyTypes[i - 1];

			bool isValid = true;
			juce::String errorMessage;

			// Last item must be SoundSFX, report this error above any others
			if(i == hierarchyTypes.size() - 1 && child != Wwise::ObjectType::SoundSFX && child != Wwise::ObjectType::SoundVoice)
			{
				isValid = false;
				errorMessage << "Last item must be of type 'SoundSFX' or 'Sound Voice'";
			}
			else if(parent != Wwise::ObjectType::Unknown &&
					child != Wwise::ObjectType::Unknown &&
					!WwiseHelper::validateObjectTypeParentChildRelationShip(parent, child))
			{
				isValid = false;
				errorMessage << "'" << WwiseHelper::objectTypeToReadableString(child) << "' cannot be a child of '" << WwiseHelper::objectTypeToReadableString(parent) << "'";
			}

			auto hierarchyMappingNode = hierarchyMapping.getChild(i - 1); // Index is off by 1 due to the importDestination
			hierarchyMappingNode.setPropertyExcludingListener(this, IDs::objectTypeValid, isValid, nullptr);
			hierarchyMappingNode.setPropertyExcludingListener(this, IDs::objectTypeErrorMessage, errorMessage, nullptr);
		}
	}
} // namespace AK::WwiseTransfer::ApplicationState
