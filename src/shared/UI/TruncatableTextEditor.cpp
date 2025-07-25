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
		juce::String text{"..."};
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
