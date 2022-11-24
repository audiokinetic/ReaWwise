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

#include "Core/WaapiClient.h"
#include "CustomDrawableButton.h"
#include "Model/Import.h"
#include "ValidatableTextEditor.h"
#include "WildcardSelector.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace AK::WwiseTransfer
{
	class SelectedRowPropertiesComponent
		: public juce::GroupComponent
		, public juce::ValueTree::Listener
		, public juce::AsyncUpdater
	{
	public:
		SelectedRowPropertiesComponent(juce::ValueTree appState, WaapiClient& waapiClient);
		~SelectedRowPropertiesComponent() override;

		void resized() override;

	private:
		juce::Label objectTypeLabel;
		juce::Label objectNameLabel;
		juce::Label propertyTemplatePathLabel;
		juce::ComboBox objectTypeComboBox;
		ValidatableTextEditor objectNameEditor;
		ValidatableTextEditor propertyTemplatePathEditor;
		juce::ToggleButton propertyTemplateToggleButton;
		WildcardSelector wildcardSelector;
		CustomDrawableButton propertyTemplatePathSyncButton;
		juce::ComboBox objectLanguageComboBox;

		juce::ValueTree applicationState;

		juce::ValueTree hierarchyMapping;
		juce::CachedValue<int> selectedRow;

		juce::ValueTree hierarchyMappingNode;
		juce::CachedValue<juce::String> objectName;
		juce::CachedValue<bool> objectNameValid;
		juce::CachedValue<juce::String> objectNameErrorMessage;
		juce::CachedValue<Wwise::ObjectType> objectType;
		juce::CachedValue<juce::String> propertyTemplatePath;
		juce::CachedValue<Wwise::ObjectType> propertyTemplatePathType;
		juce::CachedValue<bool> propertyTemplatePathEnabled;
		juce::CachedValue<bool> propertyTemplatePathValid;

		juce::CachedValue<bool> applyTemplateFeatureEnabled;

		juce::ValueTree emptyHierarchyMappingNode;

		juce::ValueTree languages;
		juce::CachedValue<juce::String> objectLanguage;

		WaapiClient& waapiClient;

		void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
		void valueTreeChildAdded(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded) override;
		void valueTreeChildRemoved(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved) override;
		void valueTreeChildOrderChanged(juce::ValueTree& parentTreeWhoseChildrenHaveMoved, int oldIndex, int newIndex) override;

		void handleAsyncUpdate() override;
		void refreshComponent();
		void updatePropertyTemplatePath();
		void updatePropertyTemplateSection();

		void paint(juce::Graphics& g) override;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SelectedRowPropertiesComponent);
	};
} // namespace AK::WwiseTransfer
