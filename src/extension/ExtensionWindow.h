#pragma once

#include "Core/DawContext.h"
#include "Theme/CustomLookAndFeel.h"

namespace AK::ReaWwise
{
	class ExtensionWindow : public juce::ResizableWindow
	{
	public:
		ExtensionWindow(WwiseTransfer::DawContext& dawContext);
		~ExtensionWindow() override;

		int getDesktopWindowStyleFlags() const override;
		void userTriedToCloseWindow() override;

	protected:
		void resized() override;

	private:
		WwiseTransfer::CustomLookAndFeel lookAndFeel;
	};
} // namespace AK::ReaWwise
