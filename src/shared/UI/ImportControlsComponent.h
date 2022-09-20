#pragma once

#include "Core/DawContext.h"
#include "Core/ImportTask.h"
#include "Core/WaapiClient.h"
#include "Persistance/ApplicationProperties.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace AK::WwiseTransfer
{
	class ImportControlsComponent
		: public juce::Component
		, private juce::ValueTree::Listener
		, private juce::AsyncUpdater
	{
	public:
		ImportControlsComponent(juce::ValueTree appState, WaapiClient& waapiClient, DawContext& dawContext, ApplicationProperties& applicationProperties, const juce::String& applicationName);
		~ImportControlsComponent() override;

		void resized() override;

		void transferToWwise();

	private:
		juce::TextButton importButton;
		juce::ValueTree applicationState;
		juce::CachedValue<bool> originalsSubfolderValid;
		juce::CachedValue<bool> importDestinationValid;
		juce::CachedValue<juce::String> importDestination;
		juce::CachedValue<juce::String> originalsSubFolder;
		juce::CachedValue<juce::String> originalsFolder;
		juce::CachedValue<juce::String> projectPath;
		juce::CachedValue<juce::String> languageSubfolder;
		juce::CachedValue<Import::ContainerNameExistsOption> containerNameExistsOption;
		juce::CachedValue<Import::ApplyTemplateOption> applyTemplateOption;
		juce::ValueTree hierarchyMapping;
		juce::ValueTree previewItems;

		juce::CachedValue<juce::String> selectObjectsOnImportCommand;
		juce::CachedValue<bool> applyTemplateFeatureEnabled;
		juce::CachedValue<bool> undoGroupFeatureEnabled;
		juce::CachedValue<bool> waqlEnabled;

		WaapiClient& waapiClient;
		DawContext& dawContext;
		ApplicationProperties& applicationProperties;

		std::unique_ptr<juce::ThreadWithProgressWindow> importTask;

		juce::ToggleButton showSilentIncrementWarningToggle;

		const juce::String applicationName;

		void showImportSummary(const Import::Summary& summary, const Import::Task::Options& importTaskOptions);
		void viewImportSummaryDetails(const Import::Summary& summary, const Import::Task::Options& importTaskOptions);

		void refreshComponent();

		void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
		void valueTreeChildAdded(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded);
		void valueTreeChildRemoved(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved);
		void valueTreeChildOrderChanged(juce::ValueTree& parentTreeWhoseChildrenHaveMoved, int oldIndex, int newIndex);

		void handleAsyncUpdate() override;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ImportControlsComponent)
	};
} // namespace AK::WwiseTransfer
