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

	private:
		void refreshComponent(bool isValid);
		juce::Value validationValue;
		juce::Value errorMessageValue;

		juce::String valueOnFocusGained;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ValidatableTextEditor);
	};
} // namespace AK::WwiseTransfer
