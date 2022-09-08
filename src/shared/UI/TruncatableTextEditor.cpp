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
		juce::String text{ "..." };
		const auto indent = getLeftIndent() * 4;
		const auto ellipsisWidth = getFont().getStringWidth(text);
		const auto width = getWidth();
		const auto trueTextSpace = width - ellipsisWidth - indent;

		if(trueTextSpace > 0)
		{
			text = valueToTruncate.getValue();
			if(text.isNotEmpty() && getFont().getStringWidth(text) > trueTextSpace)
			{
				int midPoint{};
				while(getFont().getStringWidth(text) > trueTextSpace)
				{
					midPoint = text.length() / 2;
					text = text.substring(0, midPoint) + text.substring(midPoint + 1);
				}

				text = text.substring(0, midPoint) + "..." + text.substring(midPoint);
			}
		}

		setText(text);
	}
} // namespace AK::WwiseTransfer
