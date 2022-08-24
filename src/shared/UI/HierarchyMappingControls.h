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
