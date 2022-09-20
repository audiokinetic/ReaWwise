#include "StandaloneWindow.h"

#include <limits>

namespace AK::WwiseTransfer
{
	StandaloneWindow::StandaloneWindow()
		: MainWindow(stubContext, JUCE_APPLICATION_NAME_STRING, true)
	{
	}

	StandaloneWindow::~StandaloneWindow()
	{
	}

	void StandaloneWindow::userTriedToCloseWindow()
	{
		MainWindow::userTriedToCloseWindow();

		juce::JUCEApplication::getInstance()->systemRequestedQuit();
	}
} // namespace AK::WwiseTransfer
