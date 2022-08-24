#include "ApplicationStateValidator.h"

#include "Helpers/ImportHelper.h"
#include "Helpers/WwiseHelper.h"
#include "Model/IDs.h"

namespace AK::WwiseTransfer::ApplicationState
{
	Validator::Validator(juce::ValueTree appState)
		: applicationState(appState)
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
			else if(property == IDs::importDestination || property == IDs::importDestinationType)
			{
				const juce::String importDestination = valueTree[IDs::importDestination];
				const auto importDestinationType = juce::VariantConverter<Wwise::ObjectType>::fromVar(valueTree[IDs::importDestinationType]);

				auto isValid = validateImportDestination(importDestination, importDestinationType);
				juce::String errorMessage = isValid ? "" : "Invalid import destination";

				valueTree.setPropertyExcludingListener(this, IDs::importDestinationValid, isValid, nullptr);
				valueTree.setPropertyExcludingListener(this, IDs::importDestinationErrorMessage, errorMessage, nullptr);
			}
		}
		else if(valueTree.getType() == IDs::hierarchyMappingNode)
		{
			if(property == IDs::objectType)
			{
				auto hierarchyMapping = valueTree.getParent();
				validateHierarchyMapping(hierarchyMapping);
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
			validateHierarchyMapping(parent);
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
			validateHierarchyMapping(parent);
		}
	}

	void Validator::valueTreeChildOrderChanged(juce::ValueTree& parent, int oldIndex, int newIndex)
	{
		if(parent.getType() == IDs::hierarchyMapping)
		{
			validateHierarchyMapping(parent);
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

	bool Validator::validateImportDestination(const juce::String& importDestination, Wwise::ObjectType objectType)
	{
		using namespace Wwise;

		static const std::initializer_list<ObjectType> allowedObjectTypes = {ObjectType::VirtualFolder, ObjectType::WorkUnit,
			ObjectType::RandomContainer, ObjectType::BlendContainer, ObjectType::ActorMixer, ObjectType::SwitchContainer};

		static const juce::String pathPrefix = "\\Actor-Mixer Hierarchy";

		auto allowedType = std::find(allowedObjectTypes.begin(), allowedObjectTypes.end(),
							   objectType) != allowedObjectTypes.end();

		auto allowedPathPrefix = importDestination.startsWith(pathPrefix);

		return importDestination.isNotEmpty() && !importDestination.endsWith("\\") && allowedType && allowedPathPrefix;
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

	void Validator::validateHierarchyMapping(juce::ValueTree hierarchyMapping)
	{
		// TODO: Should error be reported on parent or child?
		auto hierarchyMappingNodeList = ImportHelper::valueTreeToHierarchyMappingNodeList(hierarchyMapping);

		for(int i = 0; i < hierarchyMappingNodeList.size(); ++i)
		{
			auto& child = hierarchyMappingNodeList.at(i);

			bool isValid = true;
			juce::String errorMessage;

			// Last item must be SoundSFX, report this error above any others
			if(i == hierarchyMappingNodeList.size() - 1)
			{
				if(child.type != Wwise::ObjectType::SoundSFX && child.type != Wwise::ObjectType::SoundVoice)
				{
					isValid = false;
					errorMessage << "Last item must be of type 'SoundSFX' or 'Sound Voice'";
				}
			}

			// Since we have limited space in the tooltip, do not do any other validation if an error was already detected
			if(isValid && i != 0)
			{
				auto& parent = hierarchyMappingNodeList.at(i - 1);

				if(parent.type != Wwise::ObjectType::Unknown &&
					child.type != Wwise::ObjectType::Unknown &&
					!WwiseHelper::validateObjectTypeParentChildRelationShip(parent.type, child.type))
				{
					isValid = false;
					errorMessage << "'" << WwiseHelper::objectTypeToReadableString(child.type) << "' cannot be a child of '" << WwiseHelper::objectTypeToReadableString(parent.type) << "'";
				}
			}

			auto hierarchyMappingNode = hierarchyMapping.getChild(i);
			hierarchyMappingNode.setPropertyExcludingListener(this, IDs::objectTypeValid, isValid, nullptr);
			hierarchyMappingNode.setPropertyExcludingListener(this, IDs::objectTypeErrorMessage, errorMessage, nullptr);
		}
	}
} // namespace AK::WwiseTransfer::ApplicationState
