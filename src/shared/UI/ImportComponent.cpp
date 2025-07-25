/*----------------------------------------------------------------------------------------

Copyright (c) 2025 AUDIOKINETIC Inc.

This file is licensed to use under the license available at:
https://github.com/audiokinetic/ReaWwise/blob/main/License.txt (the "License").
You may not use this file except in compliance with the License.

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.  See the License for the
specific language governing permissions and limitations under the License.

----------------------------------------------------------------------------------------*/

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
