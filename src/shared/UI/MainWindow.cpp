/*----------------------------------------------------------------------------------------

Copyright (c) 2025 AUDIOKINETIC Inc.

This file is licensed to use under the license available at:
https://github.com/audiokinetic/ReaWwise/blob/main/License.txt (the "License").
You may not use this file except in compliance with the License.

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.  See the License for the
specific language governing permissions and limitations under the License.

----------------------------------------------------------------------------------------*/

#include "MainWindow.h"

#include <limits>

namespace AK::WwiseTransfer
{
	namespace ExtensionWindowConstants
	{
		constexpr int width = 600;
		constexpr int height = 800;
		constexpr int minWidth = 420;
		constexpr int minHeight = 650;
		constexpr int standardDPI = 96;
	} // namespace ExtensionWindowConstants

	MainWindow::MainWindow(WwiseTransfer::DawContext& dawContext, const juce::String& applicationName, bool addToDesktop)
		: juce::ResizableWindow(applicationName, addToDesktop)
	{
		using namespace ExtensionWindowConstants;

		juce::LookAndFeel::setDefaultLookAndFeel(&lookAndFeel);

		auto mainComponent = new WwiseTransfer::MainComponent(dawContext, applicationName);

#ifdef WIN32
		if(!mainComponent->hasScaleFactorOverride())
		{
			auto scaleFactor = juce::Desktop::getInstance().getDisplays().getMainDisplay().dpi / standardDPI;
			juce::Desktop::getInstance().setGlobalScaleFactor(scaleFactor);
		}
#endif

		setContentOwned(mainComponent, true);
		centreWithSize(width, height);
		setResizable(true, true);
		setResizeLimits(minWidth, minHeight, (std::numeric_limits<int>::max)(), (std::numeric_limits<int>::max)());
	}

	MainWindow::~MainWindow()
	{
		juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
	}

	int MainWindow::getDesktopWindowStyleFlags() const
	{
		return juce::ComponentPeer::windowHasCloseButton | juce::ComponentPeer::windowHasTitleBar |
		       juce::ComponentPeer::windowIsResizable | juce::ComponentPeer::windowHasMinimiseButton |
		       juce::ComponentPeer::windowAppearsOnTaskbar | juce::ComponentPeer::windowHasMaximiseButton;
	}

	void MainWindow::userTriedToCloseWindow()
	{
		setVisible(false);
	}

	void MainWindow::transferToWwise()
	{
		auto contentComponent = getContentComponent();
		if(contentComponent != nullptr)
		{
			auto mainComponent = dynamic_cast<MainComponent*>(contentComponent);
			if(mainComponent != nullptr)
				mainComponent->transferToWwise();
		}
	}
} // namespace AK::WwiseTransfer
