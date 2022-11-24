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

#include "Logger.h"

namespace AK::WwiseTransfer
{
	Logger::Logger(const juce::String& applicationName)
		: juce::FileLogger(getSystemLogFileFolder().getChildFile(applicationName)
#ifdef WIN32
							   .getChildFile("Logs") // System log file folder for windows is generic app data folder. Add subfolder for logs.
#endif
							   .getChildFile("Log_" + juce::Time::getCurrentTime().formatted("%Y-%m-%d"))
							   .withFileExtension(".txt"),
			  applicationName + " Log")
	{
	}

	void Logger::logMessage(const juce::String& message)
	{
		juce::String messageWithTimeStamp;

		messageWithTimeStamp << juce::Time::getCurrentTime().formatted("%Y-%m-%d %H:%M:%S") << " " << message;

		juce::FileLogger::logMessage(messageWithTimeStamp);

		auto onLogMessage = [&](IListener& listener)
		{
			listener.onLogMessage(messageWithTimeStamp);
		};

		listeners.call(onLogMessage);
	}

	void Logger::addListener(IListener& listener)
	{
		listeners.add(&listener);
	}

	void Logger::removeListener(IListener& listener)
	{
		listeners.remove(&listener);
	}
} // namespace AK::WwiseTransfer
