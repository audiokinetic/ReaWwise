#pragma once

#include "Core/DawContext.h"
#include "UI/MainComponent.h"

namespace AK::ReaWwise
{
	class ExtensionWindow : public juce::ResizableWindow
	{
	public:
		ExtensionWindow(WwiseTransfer::DawContext& dawContext);

		int getDesktopWindowStyleFlags() const override;
		void userTriedToCloseWindow() override;

	private:
		std::unique_ptr<WwiseTransfer::MainComponent> mainContentComponent;
		WwiseTransfer::CustomLookAndFeel lookAndFeel;
	};
} // namespace AK::ReaWwise
