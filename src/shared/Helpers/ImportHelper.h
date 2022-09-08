#pragma once

#include "Helpers/WwiseHelper.h"
#include "Model/IDs.h"
#include "Model/Import.h"

#include <AK/Tools/Common/AkFNVHash.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace AK::WwiseTransfer::ImportHelper
{
	inline juce::ValueTree hierarchyMappingNodeToValueTree(Import::HierarchyMappingNode hierarchyMappingNode)
	{
		juce::ValueTree valueTree(IDs::hierarchyMappingNode);
		valueTree.setProperty(IDs::objectName, hierarchyMappingNode.name, nullptr);
		valueTree.setProperty(IDs::objectNameValid, hierarchyMappingNode.nameValid, nullptr);
		valueTree.setProperty(IDs::objectNameErrorMessage, hierarchyMappingNode.nameErrorMessage, nullptr);
		valueTree.setProperty(IDs::objectType, juce::VariantConverter<Wwise::ObjectType>::toVar(hierarchyMappingNode.type), nullptr);
		valueTree.setProperty(IDs::objectTypeValid, hierarchyMappingNode.typeValid, nullptr);
		valueTree.setProperty(IDs::objectTypeErrorMessage, hierarchyMappingNode.typeErrorMessage, nullptr);
		valueTree.setProperty(IDs::propertyTemplatePath, hierarchyMappingNode.propertyTemplatePath, nullptr);
		valueTree.setProperty(IDs::propertyTemplatePathEnabled, hierarchyMappingNode.propertyTemplatePathEnabled, nullptr);
		valueTree.setProperty(IDs::propertyTemplatePathValid, hierarchyMappingNode.propertyTemplatePathValid, nullptr);
		valueTree.setProperty(IDs::propertyTemplatePathErrorMessage, hierarchyMappingNode.propertyTemplatePathErrorMessage, nullptr);
		valueTree.setProperty(IDs::objectLanguage, hierarchyMappingNode.language, nullptr);

		return valueTree;
	}

	inline juce::ValueTree hierachyMappingNodeListToValueTree(const std::vector<Import::HierarchyMappingNode>& hierarchyMappingNodeList)
	{
		juce::ValueTree valueTree(IDs::hierarchyMapping);
		for(const auto& hierarchyMappingNode : hierarchyMappingNodeList)
		{
			valueTree.appendChild(hierarchyMappingNodeToValueTree(hierarchyMappingNode), nullptr);
		}

		return valueTree;
	}

	inline Import::HierarchyMappingNode valueTreeToHiarchyMappingNode(juce::ValueTree valueTree)
	{
		return Import::HierarchyMappingNode(valueTree[IDs::objectName],
			valueTree[IDs::objectNameValid],
			valueTree[IDs::objectNameErrorMessage],
			juce::VariantConverter<Wwise::ObjectType>::fromVar(valueTree[IDs::objectType]),
			valueTree[IDs::objectTypeValid],
			valueTree[IDs::objectTypeErrorMessage],
			valueTree[IDs::propertyTemplatePath],
			valueTree[IDs::propertyTemplatePathEnabled],
			valueTree[IDs::propertyTemplatePathValid],
			valueTree[IDs::propertyTemplatePathErrorMessage],
			valueTree[IDs::objectLanguage]);
	}

	inline Import::PreviewItemNode valueTreeToPreviewItemNode(juce::ValueTree valueTree)
	{
		return Import::PreviewItemNode{
			valueTree[IDs::objectName],
			juce::VariantConverter<Wwise::ObjectType>::fromVar(valueTree[IDs::objectType]),
			juce::VariantConverter<Import::ObjectStatus>::fromVar(valueTree[IDs::objectStatus]),
			valueTree[IDs::audioFilePath],
			juce::VariantConverter<Import::WavStatus>::fromVar(valueTree[IDs::wavStatus]),
		};
	}

	inline juce::ValueTree previewItemNodeToValueTree(const juce::String& path, Import::PreviewItemNode previewItem)
	{
		juce::ValueTree valueTree(path);
		valueTree.setProperty(IDs::objectName, previewItem.name, nullptr);
		valueTree.setProperty(IDs::objectType, juce::VariantConverter<Wwise::ObjectType>::toVar(previewItem.type), nullptr);
		valueTree.setProperty(IDs::objectStatus, juce::VariantConverter<Import::ObjectStatus>::toVar(previewItem.objectStatus), nullptr);
		valueTree.setProperty(IDs::audioFilePath, previewItem.audioFilePath, nullptr);
		valueTree.setProperty(IDs::wavStatus, juce::VariantConverter<Import::WavStatus>::toVar(previewItem.wavStatus), nullptr);

		return valueTree;
	}

	inline std::vector<Import::HierarchyMappingNode> valueTreeToHierarchyMappingNodeList(juce::ValueTree hierarchyMappingValueTree)
	{
		std::vector<Import::HierarchyMappingNode> hierarchyMappingNodeList;

		for(int i = 0; i < hierarchyMappingValueTree.getNumChildren(); ++i)
		{
			auto hierarchyMappingNodeValueTree = hierarchyMappingValueTree.getChild(i);

			hierarchyMappingNodeList.emplace_back(ImportHelper::valueTreeToHiarchyMappingNode(hierarchyMappingNodeValueTree));
		}

		return hierarchyMappingNodeList;
	}

	inline juce::String hierarchyMappingToPath(std::vector<Import::HierarchyMappingNode> hierachyMappingNodeList)
	{
		juce::String path;

		for(auto& hierarchyMappingNode : hierachyMappingNodeList)
		{
			if(hierarchyMappingNode.type != Wwise::ObjectType::Unknown)
				path << WwiseHelper::buildObjectPathNode(hierarchyMappingNode.type, hierarchyMappingNode.name);
		}

		return path;
	}

	inline juce::String containerNameExistsOptionToString(Import::ContainerNameExistsOption option)
	{
		switch(option)
		{
		case Import::ContainerNameExistsOption::UseExisting:
			return "useExisting";
		case Import::ContainerNameExistsOption::CreateNew:
			return "createNew";
		case Import::ContainerNameExistsOption::Replace:
			return "replaceExisting";
		default:
			return "notImplemented";
		}
	}

	inline Import::ContainerNameExistsOption stringToContainerNameExistsOption(const juce::String& option)
	{
		if(option == "useExisting")
			return Import::ContainerNameExistsOption::UseExisting;
		else if(option == "createNew")
			return Import::ContainerNameExistsOption::CreateNew;
		else if(option == "replace")
			return Import::ContainerNameExistsOption::Replace;
		else
			return Import::ContainerNameExistsOption::Unknown;
	}

	inline juce::String containerNameExistsOptionToReadableString(Import::ContainerNameExistsOption option)
	{
		switch(option)
		{
		case Import::ContainerNameExistsOption::UseExisting:
			return "Use Existing";
		case Import::ContainerNameExistsOption::CreateNew:
			return "Create New";
		case Import::ContainerNameExistsOption::Replace:
			return "Replace";
		default:
			return "Not Implemented";
		}
	}

	inline juce::String audioFilenameExistsOptionToReadableString(Import::AudioFilenameExistsOption option)
	{
		switch(option)
		{
		case Import::AudioFilenameExistsOption::UseExisting:
			return "Use Existing";
		case Import::AudioFilenameExistsOption::Replace:
			return "Replace";
		default:
			return "Not Implemented";
		}
	}

	inline juce::String applyTemplateOptionToReadableString(Import::ApplyTemplateOption option)
	{
		switch(option)
		{
		case Import::ApplyTemplateOption::Always:
			return "Always";
		case Import::ApplyTemplateOption::NewObjectCreationOnly:
			return "New Object Creation Only";
		default:
			return "Not Implemented";
		}
	}

	inline juce::String objectStatusToReadableString(Import::ObjectStatus itemState)
	{
		switch(itemState)
		{
		case Import::ObjectStatus::Replaced:
			return "Replaced";
		case Import::ObjectStatus::New:
			return "New";
		case Import::ObjectStatus::NewRenamed:
			return "New (Renamed)";
		case Import::ObjectStatus::NoChange:
			return "No Change";
		default:
			return "";
		}
	}

	inline juce::String wavStatusToReadableString(Import::WavStatus itemState)
	{
		switch(itemState)
		{
		case Import::WavStatus::Replaced:
			return "Replaced";
		case Import::WavStatus::New:
			return "New";
		default:
			return "";
		}
	}

	inline unsigned int importPreviewItemsToHash(const std::vector<Import::PreviewItem>& importItems)
	{
		AK::FNVHash32 hash;

		for(const auto& importItem : importItems)
		{
			auto audioFilePathRaw = importItem.audioFilePath.toUTF8();
			hash.Compute(audioFilePathRaw, audioFilePathRaw.sizeInBytes());

			auto originalsSubfolderRaw = importItem.originalsSubFolder.toUTF8();
			hash.Compute(originalsSubfolderRaw, originalsSubfolderRaw.sizeInBytes());

			hash.Compute(AK::FNVHash32::ComputeLowerCase(importItem.path.toUTF8()));
		}

		return hash.Get();
	}
} // namespace AK::WwiseTransfer::ImportHelper
