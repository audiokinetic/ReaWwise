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

#include "ValidatableTextEditor.h"

#include "Theme/CustomLookAndFeel.h"

namespace AK::WwiseTransfer
{
	ValidatableTextEditor::ValidatableTextEditor(const juce::String& componentName)
		: juce::TextEditor(componentName)
		, validationValue(juce::var(true))
		, errorMessageValue(juce::var(""))
	{
		onEscapeKey = [this]
		{
			setText(valueOnFocusGained);
			valueOnFocusGained = "";
			moveKeyboardFocusToSibling(true);
		};

		onReturnKey = [this]
		{
			moveKeyboardFocusToSibling(true);
		};

		validationValue.addListener(this);
	}

	ValidatableTextEditor::~ValidatableTextEditor()
	{
		validationValue.removeListener(this);
	}

	juce::String ValidatableTextEditor::getTooltip()
	{
		return errorMessageValue.getValue();
	}

	void ValidatableTextEditor::focusGained(juce::Component::FocusChangeType focusChangeType)
	{
		valueOnFocusGained = getText();

		juce::TextEditor::focusGained(focusChangeType);
	}

	void ValidatableTextEditor::valueChanged(juce::Value& value)
	{
		if(value.refersToSameSourceAs(validationValue))
			refreshComponent(validationValue.getValue());
	}

	juce::Value& ValidatableTextEditor::getValidationValue()
	{
		return validationValue;
	}

	juce::Value& ValidatableTextEditor::getErrorMessageValue()
	{
		return errorMessageValue;
	}

	void ValidatableTextEditor::refreshComponent(bool isValid)
	{
		auto outlineColour = getLookAndFeel().findColour(juce::TextEditor::outlineColourId);

		if(!isValid)
			outlineColour = getLookAndFeel().findColour(ValidatableTextEditor::errorOutlineColor);

		setColour(juce::TextEditor::outlineColourId, outlineColour);
	}
} // namespace AK::WwiseTransfer
