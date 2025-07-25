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

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace AK::WwiseTransfer
{
	class Logger : public juce::FileLogger
	{
	public:
		Logger(const juce::String& applicationName);

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
		juce::ListenerList<IListener> listeners;
	};
} // namespace AK::WwiseTransfer
