#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace AK::WwiseTransfer
{
	class LoadingComponent : public juce::Component
	{
	public:
		LoadingComponent();

		void resized() override;
		void paint(juce::Graphics& g) override;

	private:
		juce::ProgressBar progressBar;
		juce::Label text;

		double progress = -1;
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LoadingComponent)
	};
} // namespace AK::WwiseTransfer
