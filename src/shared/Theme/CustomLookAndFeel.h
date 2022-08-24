#pragma once

#include "Model/Import.h"
#include "Model/Wwise.h"

#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>

namespace AK::WwiseTransfer
{
	namespace CustomLookAndFeelConstants
	{
		constexpr float smallFontSize = 16.0f;
		constexpr float regularFontSize = 18.0f;
		constexpr float largeFontSize = 20.0f;
	}; // namespace CustomLookAndFeelConstants

	class CustomLookAndFeel : public juce::LookAndFeel_V4
	{
	public:
		CustomLookAndFeel();

		const std::shared_ptr<juce::Drawable>& getIconForObjectType(Wwise::ObjectType objectType);
		juce::Colour getTextColourForObjectStatus(Import::ObjectStatus objectStatus);

		void drawTextEditorOutline(juce::Graphics& g, int width, int height, juce::TextEditor& textEditor) override;

		void drawTableHeaderColumn(juce::Graphics& g, juce::TableHeaderComponent& header,
			const juce::String& columnName, int /*columnId*/,
			int width, int height, bool isMouseOver, bool isMouseDown,
			int columnFlags) override;

		void drawGroupComponentOutline(juce::Graphics&, int w, int h, const juce::String& text,
			const juce::Justification&, juce::GroupComponent&) override;

		void drawTooltip(juce::Graphics&, const juce::String& text, int w, int h) override;

		juce::Typeface::Ptr getTypefaceForFont(const juce::Font&) override;

		juce::Font getLabelFont(juce::Label& label) override;
		juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override;
		juce::Font getPopupMenuFont() override;
		juce::Font getAlertWindowMessageFont() override;
		juce::Font getAlertWindowFont() override;
		juce::Font getAlertWindowTitleFont() override;
		juce::Font getTableHeaderFont();

	private:
		juce::Typeface::Ptr regularTypeFace;
		juce::Typeface::Ptr boldTypeFace;

		juce::Colour windowBackgroundColor;
		juce::Colour widgetBackgroundColor;
		juce::Colour thickOutlineColor;
		juce::Colour textColor;
		juce::Colour textColorDisabled;
		juce::Colour highlightedTextColor;
		juce::Colour highlightedFillColor;
		juce::Colour buttonBackgroundColor;
		juce::Colour thinOutlineColor;
		juce::Colour tableHeaderBackgroundColor;
		juce::Colour previewItemNoChangeColor;
		juce::Colour previewItemNewColor;
		juce::Colour previewItemReplacedColor;
	};
} // namespace AK::WwiseTransfer
