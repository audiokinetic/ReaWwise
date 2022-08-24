#pragma once

#include "AboutComponent.h"
#include "CustomDrawableButton.h"
#include "TruncatableTextEditor.h"
#include "ValidatableTextEditor.h"
#include "WildcardSelector.h"

#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>

namespace AK::WwiseTransfer
{
	class OriginalsSubfolderComponent
		: public juce::Component
		, private juce::ValueTree::Listener
		, private juce::AsyncUpdater
	{
	public:
		OriginalsSubfolderComponent(juce::ValueTree applicationState, const juce::String& applicationName);
		~OriginalsSubfolderComponent();

	private:
		juce::ValueTree applicationState;
		juce::CachedValue<juce::String> projectPath;
		juce::CachedValue<juce::String> originalsSubfolder;
		juce::CachedValue<juce::String> originalsFolder;
		juce::CachedValue<juce::String> languageSubfolder;
		juce::Label projectPathLabel;
		juce::Label originalsSubfolderLabel;
		TruncatableTextEditor projectPathEditor;
		ValidatableTextEditor originalsSubfolderEditor;

		CustomDrawableButton fileBrowserButton;
		WildcardSelector wildcardSelector;

		std::unique_ptr<juce::FileChooser> fileChooser;

		CustomDrawableButton aboutButton;
		AboutComponent aboutComponent;

		void resized() override;
		void refreshComponent();
		void selectOriginalsSubfoler();
		void onWildcardSelected(const juce::String& wildcard);
		void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
		void handleAsyncUpdate() override;

		void showAboutWindow();

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OriginalsSubfolderComponent)
	};
} // namespace AK::WwiseTransfer
