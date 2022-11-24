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

#include <juce_gui_basics/juce_gui_basics.h>

namespace AK::WwiseTransfer
{
	class ValidatableTextEditor
		: public juce::TextEditor
		, public juce::Value::Listener
	{
	public:
		explicit ValidatableTextEditor(const juce::String& componentName = juce::String());
		~ValidatableTextEditor();

		void valueChanged(juce::Value& value) override;
		juce::String getTooltip() override;

		void focusGained(juce::Component::FocusChangeType focusChangeType) override;

		juce::Value& getValidationValue();
		juce::Value& getErrorMessageValue();

		enum ColourIds
		{
			errorOutlineColor = 0x00000000,
		};

	private:
		void refreshComponent(bool isValid);
		juce::Value validationValue;
		juce::Value errorMessageValue;

		juce::String valueOnFocusGained;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ValidatableTextEditor);
	};
} // namespace AK::WwiseTransfer
