/*----------------------------------------------------------------------------------------

Copyright (c) 2023 AUDIOKINETIC Inc.

This file is licensed to use under the license available at:
https://github.com/audiokinetic/ReaWwise/blob/main/License.txt (the "License").
You may not use this file except in compliance with the License.

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.  See the License for the
specific language governing permissions and limitations under the License.

----------------------------------------------------------------------------------------*/

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
