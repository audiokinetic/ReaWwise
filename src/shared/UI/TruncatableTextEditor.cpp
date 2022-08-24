#include "TruncatableTextEditor.h"

namespace AK::WwiseTransfer
{
	TruncatableTextEditor::TruncatableTextEditor()
		: valueToTruncate(juce::var(""))
	{
		valueToTruncate.addListener(this);
	}

	TruncatableTextEditor::~TruncatableTextEditor()
	{
		valueToTruncate.removeListener(this);
	}

	void TruncatableTextEditor::valueChanged(juce::Value& value)
	{
		resetText();
	}

	juce::Value& TruncatableTextEditor::getValueToTruncate()
	{
		return valueToTruncate;
	};

	void TruncatableTextEditor::resized()
	{
		juce::TextEditor::resized();

		resetText();
	}

	juce::String TruncatableTextEditor::getTooltip()
	{
		return valueToTruncate.getValue();
	}

	void TruncatableTextEditor::resetText()
	{
		auto indent = getLeftIndent() * 4;

		juce::String text = valueToTruncate.getValue();

		auto trueTextSpace = getWidth() - juce::Font().getStringWidth("...") - indent;

		if(text.isNotEmpty() && juce::Font().getStringWidth(text) > trueTextSpace)
		{
			auto midPoint = 0;
			while(juce::Font().getStringWidth(text) > trueTextSpace)
			{
				midPoint = juce::roundFloatToInt(float(text.length()) / 2.0f);
				text = text.substring(0, midPoint) + text.substring(midPoint + 1);
			}

			text = text.substring(0, midPoint) + "..." + text.substring(midPoint);
		}

		setText(text);
	};
} // namespace AK::WwiseTransfer
