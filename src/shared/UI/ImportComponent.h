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
