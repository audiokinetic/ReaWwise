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

#include "Standalone.h"

#include "StandaloneWindow.h"

namespace AK::WwiseTransfer
{
	const juce::String Standalone::getApplicationName()
	{
		return JUCE_APPLICATION_NAME_STRING;
	}

	const juce::String Standalone::getApplicationVersion()
	{
		return JUCE_APPLICATION_VERSION_STRING;
	}

	bool Standalone::moreThanOneInstanceAllowed()
	{
		return false;
	}

	void Standalone::initialise(const juce::String& commandLine)
	{
		juce::ignoreUnused(commandLine);
		mainWindow.reset(new StandaloneWindow());
		mainWindow->setVisible(true);
	}

	void Standalone::shutdown()
	{
		mainWindow.reset();
	}

	START_JUCE_APPLICATION(Standalone)
} // namespace AK::WwiseTransfer
