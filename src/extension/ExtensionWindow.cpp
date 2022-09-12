#include "ExtensionWindow.h"
#include "UI/MainComponent.h"

#include <limits>

namespace AK::ReaWwise
{
	namespace ExtensionWindowConstants
	{
		constexpr int width = 600;
		constexpr int height = 800;
		constexpr int minWidth = 420;
		constexpr int minHeight = 650;
		constexpr int standardDPI = 96;
	} // namespace ExtensionWindowConstants

	ExtensionWindow::ExtensionWindow(WwiseTransfer::DawContext& dawContext)
		: juce::ResizableWindow(JUCE_APPLICATION_NAME_STRING, false)
	{
		using namespace ExtensionWindowConstants;

		juce::LookAndFeel::setDefaultLookAndFeel(&lookAndFeel);

		auto mainContentComponent = new WwiseTransfer::MainComponent(dawContext, JUCE_APPLICATION_NAME_STRING);

#ifdef WIN32
		if(!mainContentComponent->hasScaleFactorOverride())
		{
			auto scaleFactor = juce::Desktop::getInstance().getDisplays().getMainDisplay().dpi / standardDPI;
			juce::Desktop::getInstance().setGlobalScaleFactor(scaleFactor);
		}
#endif

		setContentOwned(mainContentComponent, true);
		centreWithSize(width, height);
		setResizable(true, true);
		setResizeLimits(minWidth, minHeight, (std::numeric_limits<int>::max)(), (std::numeric_limits<int>::max)());
	}

	ExtensionWindow::~ExtensionWindow()
	{
		juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
	}

	int ExtensionWindow::getDesktopWindowStyleFlags() const
	{
		return juce::ComponentPeer::windowHasCloseButton | juce::ComponentPeer::windowHasTitleBar |
		       juce::ComponentPeer::windowIsResizable | juce::ComponentPeer::windowHasMinimiseButton |
		       juce::ComponentPeer::windowAppearsOnTaskbar | juce::ComponentPeer::windowHasMaximiseButton;
	}

	void ExtensionWindow::userTriedToCloseWindow()
	{
		setVisible(false);
	}

	void ExtensionWindow::resized()
	{
		juce::ResizableWindow::resized();
	}

} // namespace AK::ReaWwise
