#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace AK::WwiseTransfer
{
	class TruncatableTextEditor
		: public juce::TextEditor
		, public juce::Value::Listener
	{
	public:
		TruncatableTextEditor();
		~TruncatableTextEditor();

		void valueChanged(juce::Value& value) override;
		juce::Value& getValueToTruncate();
		void resized() override;
		juce::String getTooltip() override;

	private:
		juce::Value valueToTruncate;
		juce::String lastValue;

		void resetText();

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TruncatableTextEditor);
	};
} // namespace AK::WwiseTransfer
