#pragma once

#include "StubContext.h"
#include "UI/MainComponent.h"

namespace AK::WwiseTransfer
{
	class StandaloneWindow : public juce::ResizableWindow
	{
	public:
		StandaloneWindow();
		~StandaloneWindow() override;

		int getDesktopWindowStyleFlags() const override;
		void userTriedToCloseWindow() override;

	private:
		std::unique_ptr<MainComponent> mainContentComponent;
		CustomLookAndFeel lookAndFeel;
		StubContext stubContext;
	};
} // namespace AK::WwiseTransfer
