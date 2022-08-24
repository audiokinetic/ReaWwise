#include "LoadingComponent.h"

namespace AK::WwiseTransfer
{
	namespace LoadingComponentConstants
	{
		constexpr int loaderSize = 25;
		constexpr int textHeight = 24;
		constexpr float backgroundOpacity = 0.8f;
	} // namespace LoadingComponentConstants

	LoadingComponent::LoadingComponent()
		: progressBar(progress)
	{
		text.setText("Loading preview ...", juce::dontSendNotification);
		text.setJustificationType(juce::Justification::centred);

		addAndMakeVisible(progressBar);
		addAndMakeVisible(text);
	}

	void LoadingComponent::resized()
	{
		using namespace LoadingComponentConstants;

		auto area = getLocalBounds();

		juce::FlexBox fb;
		fb.flexDirection = juce::FlexBox::Direction::column;
		fb.alignItems = juce::FlexBox::AlignItems::center;
		fb.justifyContent = juce::FlexBox::JustifyContent::center;

		fb.items.add(juce::FlexItem(progressBar).withWidth(loaderSize).withHeight(loaderSize));
		fb.items.add(juce::FlexItem(text).withWidth(area.getWidth()).withHeight(textHeight));

		fb.performLayout(area);
	}

	void LoadingComponent::paint(juce::Graphics& g)
	{
		auto color = getLookAndFeel()
		                 .findColour(juce::ResizableWindow::ColourIds::backgroundColourId)
		                 .withAlpha(LoadingComponentConstants::backgroundOpacity);

		g.fillAll(color);
	}
} // namespace AK::WwiseTransfer
