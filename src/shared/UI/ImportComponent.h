#pragma once

#include "Core/WaapiClient.h"
#include "HierarchyMappingControls.h"
#include "HierarchyMappingTable.h"
#include "Model/Import.h"
#include "Persistance/ApplicationProperties.h"
#include "PresetMenuComponent.h"
#include "SelectedRowPropertiesComponent.h"
#include "SimpleListBox.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace AK::WwiseTransfer
{
	class ImportComponent
		: public juce::GroupComponent
	{
	public:
		ImportComponent(juce::ValueTree appState, WaapiClient& waapiClient, ApplicationProperties& applicationProperties, const juce::String& applicationName);

		void resized() override;

	private:
		juce::Label hierarchyMappingLabel;
		HierarchyMappingTable hierarchyMappingTable;
		HierarchyMappingControls hierarchyMappingControls;

		SelectedRowPropertiesComponent selectedRowPropertiesComponent;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ImportComponent);
	};
} // namespace AK::WwiseTransfer
