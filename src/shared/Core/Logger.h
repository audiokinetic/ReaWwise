/*----------------------------------------------------------------------------------------

Copyright (c) 2026 AUDIOKINETIC Inc.

This file is licensed to use under the license available at:
https://github.com/audiokinetic/ReaWwise/blob/main/License.txt (the "License").
You may not use this file except in compliance with the License.

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.  See the License for the
specific language governing permissions and limitations under the License.

----------------------------------------------------------------------------------------*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace AK::WwiseTransfer
{
	class Logger : public juce::FileLogger
	{
	public:
		Logger(const juce::String& applicationName);

		Logger(const juce::File& logFile, const juce::String& welcomeMessage, juce::int64 maxFileSizeBytes, juce::int64 trimToBytes);

		class IListener
		{
		public:
			virtual ~IListener() = default;

			virtual void onLogMessage(const juce::String& message) = 0;
		};

		void addListener(IListener& listener);
		void removeListener(IListener& listener);

	protected:
		void logMessage(const juce::String& message) override;

	private:
		void trimLogFile();

		juce::ListenerList<IListener> listeners;

		juce::CriticalSection writeLock; // Guards the members below, and makes trimming and writing atomic.
		const juce::int64 maxFileSizeBytes;
		const juce::int64 trimToBytes;
		juce::int64 bytesLogged = 0;
		juce::int64 nextTrimAtBytes;
	};
} // namespace AK::WwiseTransfer
