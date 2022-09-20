#pragma once

#include "StubContext.h"

#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>

namespace AK::WwiseTransfer
{
	class StandaloneWindow;

	class Standalone : public juce::JUCEApplication
	{
	public:
		const juce::String getApplicationName() override;
		const juce::String getApplicationVersion() override;
		bool moreThanOneInstanceAllowed() override;
		void initialise(const juce::String& commandLine) override;
		void shutdown() override;

	private:
		std::unique_ptr<StandaloneWindow> mainWindow;
	};
} // namespace AK::WwiseTransfer
