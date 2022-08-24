#pragma once

#include "Helpers/ImportHelper.h"
#include "Model/IDs.h"
#include "Model/Import.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace AK::WwiseTransfer::PersistanceHelper
{
	inline juce::String hierarchyMappingToPresetData(juce::ValueTree hierarchyMapping)
	{
		juce::ValueTree hierarchyMappingCopy = hierarchyMapping.createCopy();
		hierarchyMappingCopy.removeProperty(IDs::selectedRow, nullptr);

		for(int i = 0; i < hierarchyMappingCopy.getNumChildren(); ++i)
		{
			auto hierarchyMappingNodeCopy = hierarchyMappingCopy.getChild(i);
			hierarchyMappingNodeCopy.removeProperty(IDs::objectTypeValid, nullptr);
			hierarchyMappingNodeCopy.removeProperty(IDs::objectTypeErrorMessage, nullptr);
			hierarchyMappingNodeCopy.removeProperty(IDs::propertyTemplatePathValid, nullptr);
			hierarchyMappingNodeCopy.removeProperty(IDs::propertyTemplatePathErrorMessage, nullptr);
		}

		return hierarchyMappingCopy.toXmlString();
	}

	inline juce::ValueTree presetDataToHierarchyMapping(const juce::String& presetData)
	{
		juce::ValueTree hierarchyMappingFromPresetData = juce::ValueTree::fromXml(presetData);

		if(!hierarchyMappingFromPresetData.isValid() || hierarchyMappingFromPresetData.getType() != IDs::hierarchyMapping)
			return {};

		std::vector<Import::HierarchyMappingNode> hierarchyMapping;

		for(int i = 0; i < hierarchyMappingFromPresetData.getNumChildren(); ++i)
		{
			auto hierarchyMappingNodeFromPresetData = hierarchyMappingFromPresetData.getChild(i);

			hierarchyMapping.emplace_back(hierarchyMappingNodeFromPresetData[IDs::objectName],
				true,
				"",
				juce::VariantConverter<Wwise::ObjectType>::fromVar(hierarchyMappingNodeFromPresetData[IDs::objectType]),
				true,
				"",
				hierarchyMappingNodeFromPresetData[IDs::propertyTemplatePath],
				hierarchyMappingNodeFromPresetData[IDs::propertyTemplatePathEnabled],
				true,
				"",
				hierarchyMappingNodeFromPresetData[IDs::objectLanguage]);
		}

		auto hierarchyMappingValueTree = ImportHelper::hierachyMappingNodeListToValueTree(hierarchyMapping);
		return hierarchyMappingValueTree;
	}
} // namespace AK::WwiseTransfer::PersistanceHelper
