#pragma once

#include "Core/DawContext.h"
#include "MainComponent.h"
#include "Theme/CustomLookAndFeel.h"

namespace AK::WwiseTransfer
{
	class MainWindow : public juce::ResizableWindow
	{
	public:
		MainWindow(WwiseTransfer::DawContext& dawContext, const juce::String& applicationName, bool addToDesktop);
		~MainWindow() override;

		int getDesktopWindowStyleFlags() const override;
		void userTriedToCloseWindow() override;

		void transferToWwise();

	private:
		WwiseTransfer::CustomLookAndFeel lookAndFeel;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
	};
} // namespace AK::WwiseTransfer
