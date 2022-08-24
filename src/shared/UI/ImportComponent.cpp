#include "ImportComponent.h"

#include "Model/IDs.h"
#include "Model/Import.h"

namespace AK::WwiseTransfer
{
	namespace ImportComponentConstants
	{
		constexpr int margin = 10;
		constexpr int spacing = 4;
		constexpr int marginHorizontal = 12;
		constexpr int topMargin = 20;
		constexpr int hierarchyMappingTableHeight = 124;
		constexpr int selectedRowPropertiesSectionHeight = 134;
		constexpr int hierarchyMappingControlsWidth = 24;
	} // namespace ImportComponentConstants

	ImportComponent::ImportComponent(juce::ValueTree appState, WaapiClient& waapiClient, ApplicationProperties& applicationProperties, const juce::String& applicationName)
		: hierarchyMappingTable(appState)
		, hierarchyMappingControls(appState, applicationProperties, applicationName)
		, selectedRowPropertiesComponent(appState, waapiClient)
	{
		setText("Wwise Structures");

		addAndMakeVisible(hierarchyMappingTable);
		addAndMakeVisible(hierarchyMappingControls);
		addAndMakeVisible(selectedRowPropertiesComponent);
	}

	void ImportComponent::resized()
	{
		using namespace ImportComponentConstants;

		auto area = getLocalBounds();

		area.reduce(marginHorizontal, 0);
		area.removeFromTop(topMargin);
		area.removeFromBottom(margin);

		auto hierarchyMappingTableArea = area.removeFromTop(hierarchyMappingTableHeight);
		{
			hierarchyMappingControls.setBounds(hierarchyMappingTableArea.removeFromRight(hierarchyMappingControlsWidth));
			hierarchyMappingTableArea.removeFromRight(spacing);

			hierarchyMappingTable.setBounds(hierarchyMappingTableArea);
		}

		area.removeFromTop(spacing);
		selectedRowPropertiesComponent.setBounds(area.removeFromTop(selectedRowPropertiesSectionHeight));
	}
} // namespace AK::WwiseTransfer
