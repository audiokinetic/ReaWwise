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

#include "Model/Import.h"
#include "Model/Wwise.h"

#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>

namespace AK::WwiseTransfer
{
	namespace CustomLookAndFeelConstants
	{
		constexpr float extraSmallFontSize = 15.0f;
		constexpr float smallFontSize = 16.0f;
		constexpr float regularFontSize = 18.0f;
		constexpr float largeFontSize = 20.0f;
	}; // namespace CustomLookAndFeelConstants

	class CustomLookAndFeel : public juce::LookAndFeel_V4
	{
	public:
		CustomLookAndFeel();

		std::unique_ptr<juce::Drawable> getIconForObjectType(Wwise::ObjectType objectType);
		juce::Colour getTextColourForObjectStatus(Import::ObjectStatus objectStatus);

		void drawTextEditorOutline(juce::Graphics& g, int width, int height, juce::TextEditor& textEditor) override;

		void fillTextEditorBackground(juce::Graphics& g, int width, int height, juce::TextEditor& textEditor) override;

		void drawTableHeaderColumn(juce::Graphics& g, juce::TableHeaderComponent& header,
			const juce::String& columnName, int /*columnId*/,
			int width, int height, bool isMouseOver, bool isMouseDown,
			int columnFlags) override;

		void drawGroupComponentOutline(juce::Graphics&, int w, int h, const juce::String& text,
			const juce::Justification&, juce::GroupComponent&) override;

		void drawTooltip(juce::Graphics&, const juce::String& text, int w, int h) override;

		void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown, int buttonX,
			int buttonY, int buttonW, int buttonH, juce::ComboBox& box) override;

		juce::Typeface::Ptr getTypefaceForFont(const juce::Font&) override;

		juce::Font getLabelFont(juce::Label& label) override;
		juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override;
		juce::Font getPopupMenuFont() override;
		juce::Font getAlertWindowMessageFont() override;
		juce::Font getAlertWindowFont() override;
		juce::Font getAlertWindowTitleFont() override;
		juce::Font getComboBoxFont(juce::ComboBox&) override;
		juce::Font getTableHeaderFont();

	private:
		juce::Typeface::Ptr regularTypeFace;

		juce::Colour windowBackgroundColor;
		juce::Colour widgetBackgroundColor;
		juce::Colour thickOutlineColor;
		juce::Colour textColor;
		juce::Colour textColorDisabled;
		juce::Colour highlightedTextColor;
		juce::Colour highlightedFillColor;
		juce::Colour buttonBackgroundColor;
		juce::Colour thinOutlineColor;
		juce::Colour focusedOutlineColor;
		juce::Colour tableHeaderBackgroundColor;
		juce::Colour previewItemNoChangeColor;
		juce::Colour previewItemNewColor;
		juce::Colour previewItemReplacedColor;
		juce::Colour errorColor;
	};
} // namespace AK::WwiseTransfer
