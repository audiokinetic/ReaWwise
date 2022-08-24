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
