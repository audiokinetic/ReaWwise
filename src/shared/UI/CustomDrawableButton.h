#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace AK::WwiseTransfer
{
	class CustomDrawableButton
		: public juce::DrawableButton
	{
	public:
		CustomDrawableButton(const juce::String& buttonName, std::unique_ptr<juce::Drawable> drawable);

		juce::Rectangle<float> getImageBounds() const override;

	private:
		std::unique_ptr<juce::Drawable> drawable;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomDrawableButton)
	};
} // namespace AK::WwiseTransfer
