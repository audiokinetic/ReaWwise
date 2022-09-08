#include "StandaloneWindow.h"

#include <limits>

namespace AK::WwiseTransfer
{
	namespace StandaloneWindowConstants
	{
		constexpr int width = 600;
		constexpr int height = 800;
		constexpr int minWidth = 420;
		constexpr int minHeight = 650;
	} // namespace StandaloneWindowConstants

	StandaloneWindow::StandaloneWindow()
		: juce::ResizableWindow(JUCE_APPLICATION_NAME_STRING, true)
	{
		using namespace StandaloneWindowConstants;

		juce::LookAndFeel::setDefaultLookAndFeel(&lookAndFeel);

		mainContentComponent.reset(new MainComponent(stubContext, JUCE_APPLICATION_NAME_STRING));

		setContentNonOwned(mainContentComponent.get(), true);
		centreWithSize(width, height);
		setResizable(true, true);
		setResizeLimits(minWidth, minHeight, (std::numeric_limits<int>::max)(), (std::numeric_limits<int>::max)());
		setVisible(true);
	}

	StandaloneWindow::~StandaloneWindow()
	{
		juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
	}

	int StandaloneWindow::getDesktopWindowStyleFlags() const
	{
		return juce::ComponentPeer::windowHasCloseButton | juce::ComponentPeer::windowHasTitleBar |
		       juce::ComponentPeer::windowIsResizable | juce::ComponentPeer::windowHasMinimiseButton |
		       juce::ComponentPeer::windowAppearsOnTaskbar | juce::ComponentPeer::windowHasMaximiseButton;
	}

	void StandaloneWindow::userTriedToCloseWindow()
	{
		setVisible(false);
		juce::JUCEApplication::getInstance()->systemRequestedQuit();
	}
} // namespace AK::WwiseTransfer
