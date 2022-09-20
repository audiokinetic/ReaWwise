#pragma once

#include "Theme/CustomLookAndFeel.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace AK::WwiseTransfer
{
	class AboutComponent : public juce::Component
	{
	public:
		AboutComponent(const juce::String& applicationName);
		~AboutComponent() override;

		void resized() override;
		void paint(juce::Graphics& g);

	private:
		CustomLookAndFeel lookAndFeel;

		juce::Label titleLabel;
		juce::Label versionLabel;
		juce::Label copyrightLabel;

		juce::String copyRightSymbol;

		juce::HyperlinkButton documentationLink;
		juce::HyperlinkButton releaseNotesLink;
		juce::HyperlinkButton licensingLink;

		juce::TooltipWindow tooltipWindow;

		std::unique_ptr<juce::Drawable> wwiseIcon;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AboutComponent)
	};
} // namespace AK::WwiseTransfer
