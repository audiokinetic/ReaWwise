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
		juce::CachedValue<bool> transferInProgress;
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

		juce::TextEditor errorMessageContainer;

		const juce::String applicationName;

		void showImportSummaryModal(const Import::Summary& summary, const Import::Task::Options& importTaskOptions);
		juce::URL createImportSummaryFile(const Import::Summary& summary, const Import::Task::Options& importTaskOptions);

		void refreshComponent();

		void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
		void valueTreeChildAdded(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded);
		void valueTreeChildRemoved(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved);
		void valueTreeChildOrderChanged(juce::ValueTree& parentTreeWhoseChildrenHaveMoved, int oldIndex, int newIndex);

		void handleAsyncUpdate() override;

		void onRenderFailedDetected();
		void onImportCancelled();
		void onFileRenamedDetected(bool isPathIncomplete, const std::vector<Import::Item>& importItems);
		void onPathIncompleteDetected(const std::vector<Import::Item>& importItems);
		void onImport(const std::vector<Import::Item>& importItems);

		bool validateFullImportPathBeforeTransfer() const;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ImportControlsComponent)
	};
} // namespace AK::WwiseTransfer
