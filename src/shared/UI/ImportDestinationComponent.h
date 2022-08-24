#pragma once

#include "Core/WaapiClient.h"
#include "CustomDrawableButton.h"
#include "Model/Wwise.h"
#include "ValidatableTextEditor.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace AK::WwiseTransfer
{
	class ImportDestinationComponent
		: public juce::Component
		, private juce::ValueTree::Listener
		, private juce::AsyncUpdater
	{
	public:
		ImportDestinationComponent(juce::ValueTree applicationState, WaapiClient& waapiClient);
		~ImportDestinationComponent();

	private:
		juce::ValueTree applicationState;
		juce::CachedValue<juce::String> importDestination;
		juce::CachedValue<juce::String> projectPath;
		juce::CachedValue<Wwise::ObjectType> importDestinationType;
		juce::Label importDestinationLabel;
		ValidatableTextEditor importDestinationEditor;

		CustomDrawableButton updateImportDestinationButton;

		WaapiClient& waapiClient;

		void resized() override;
		void refreshComponent();
		void updateImportDestination();
		void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
		void handleAsyncUpdate() override;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ImportDestinationComponent);
	};
} // namespace AK::WwiseTransfer
