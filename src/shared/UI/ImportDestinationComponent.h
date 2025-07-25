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

		juce::DrawableComposite objectTypeIconComposite;
		std::unique_ptr<juce::Drawable> objectTypeIcon;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ImportDestinationComponent);
	};
} // namespace AK::WwiseTransfer
