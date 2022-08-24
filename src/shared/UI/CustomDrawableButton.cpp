#include "CustomDrawableButton.h"

namespace AK::WwiseTransfer
{
	namespace CustomDrawableButtonConstants
	{
		constexpr int margin = 2;
	}

	CustomDrawableButton::CustomDrawableButton(const juce::String& buttonName, std::unique_ptr<juce::Drawable> drawableValue)
		: juce::DrawableButton(buttonName, juce::DrawableButton::ButtonStyle::ImageOnButtonBackground)
		, drawable(std::move(drawableValue))
	{
		setImages(drawable.get());
	}

	juce::Rectangle<float> CustomDrawableButton::getImageBounds() const
	{
		return getLocalBounds().reduced(CustomDrawableButtonConstants::margin).toFloat();
	}
} // namespace AK::WwiseTransfer
