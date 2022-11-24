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
		juce::Label applyTemplateLabel;

		juce::ComboBox containerNameExistsComboBox;
		juce::ComboBox applyTemplateComboBox;

		juce::ValueTree applicationState;
		juce::CachedValue<Import::ContainerNameExistsOption> containerNameExists;
		juce::CachedValue<Import::AudioFilenameExistsOption> audioFilenameExists;
		juce::CachedValue<Import::ApplyTemplateOption> applyTemplate;

		juce::CachedValue<bool> applyTemplateFeatureEnabled;

		juce::CachedValue<juce::String> selectObjectsOnImportCommand;

		void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
		void handleAsyncUpdate() override;
		void refreshComponent();

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ImportConflictsComponent);
	};
} // namespace AK::WwiseTransfer
