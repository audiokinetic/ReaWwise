#pragma once

#include "Core/Logger.h"
#include "Core/WaapiClient.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace AK::WwiseTransfer
{
	class OutputLogComponent
		: public juce::TextEditor
		, public Logger::IListener
	{
	public:
		OutputLogComponent();

		void onLogMessage(const juce::String& logMessage);

	private:
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OutputLogComponent);
	};
} // namespace AK::WwiseTransfer
