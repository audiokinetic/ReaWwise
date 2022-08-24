#pragma once

#include "Model/Import.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace AK::WwiseTransfer
{
	class ImportConflictsComponent
		: public juce::Component
		, private juce::ValueTree::Listener
		, private juce::AsyncUpdater
	{
	public:
		ImportConflictsComponent(juce::ValueTree applicationState);
		~ImportConflictsComponent();

		void resized() override;

	private:
		juce::Label containerNameExistsLabel;
		juce::Label audioFileNameExistsLabel;
		juce::Label applyTemplateLabel;

		juce::ComboBox containerNameExistsComboBox;
		juce::ComboBox audioFileNameExistsComboBox;
		juce::ComboBox applyTemplateComboBox;

		juce::ValueTree applicationState;
		juce::CachedValue<Import::ContainerNameExistsOption> containerNameExists;
		juce::CachedValue<Import::AudioFilenameExistsOption> audioFilenameExists;
		juce::CachedValue<Import::ApplyTemplateOption> applyTemplate;

		juce::CachedValue<bool> applyTemplateFeatureEnabled;

		juce::CachedValue<juce::String> selectObjectsOnImportCommand;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ImportConflictsComponent);

		void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
		void handleAsyncUpdate() override;
		void refreshComponent();
	};
} // namespace AK::WwiseTransfer
