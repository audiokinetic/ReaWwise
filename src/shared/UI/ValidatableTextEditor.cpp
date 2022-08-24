#include "ValidatableTextEditor.h"

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
		auto outlineColour = juce::LookAndFeel::getDefaultLookAndFeel().findColour(juce::TextEditor::outlineColourId);

		if(!isValid)
		{
			outlineColour = juce::Colours::crimson;
		}

		setColour(juce::TextEditor::outlineColourId, outlineColour);
	}
} // namespace AK::WwiseTransfer
