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

#pragma once

#include "CustomDrawableButton.h"
#include "Model/Import.h"
#include "PresetMenuComponent.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace AK::WwiseTransfer
{
	class HierarchyMappingControls
		: public juce::Component
		, private juce::ValueTree::Listener
		, private juce::AsyncUpdater
	{
	public:
		HierarchyMappingControls(juce::ValueTree appState, ApplicationProperties& applicationProperties, const juce::String& applicationName);
		~HierarchyMappingControls();

		void resized() override;

	private:
		juce::ValueTree applicationState;
		juce::ValueTree hierarchyMapping;
		juce::CachedValue<int> selectedRow;

		CustomDrawableButton insertButton;
		CustomDrawableButton removeButton;
		CustomDrawableButton moveUpButton;
		CustomDrawableButton moveDownButton;

		PresetMenuComponent presetMenuComponent;

		void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
		void valueTreeChildAdded(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded) override;
		void valueTreeChildRemoved(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved) override;

		void handleAsyncUpdate() override;
		void refreshComponent();

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HierarchyMappingControls);
	};
} // namespace AK::WwiseTransfer
