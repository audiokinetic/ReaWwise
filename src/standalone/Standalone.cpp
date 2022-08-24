#include "Standalone.h"

#include "StandaloneWindow.h"

namespace AK::WwiseTransfer
{
	const juce::String ReaperWwiseTransferStandalone::getApplicationName()
	{
		return JUCE_APPLICATION_NAME_STRING;
	}

	const juce::String ReaperWwiseTransferStandalone::getApplicationVersion()
	{
		return JUCE_APPLICATION_VERSION_STRING;
	}

	bool ReaperWwiseTransferStandalone::moreThanOneInstanceAllowed()
	{
		return false;
	}

	void ReaperWwiseTransferStandalone::initialise(const juce::String& commandLine)
	{
		juce::ignoreUnused(commandLine);
		mainWindow.reset(new StandaloneWindow());
	}

	void ReaperWwiseTransferStandalone::shutdown()
	{
		mainWindow.reset();
	}

	START_JUCE_APPLICATION(ReaperWwiseTransferStandalone)
} // namespace AK::WwiseTransfer
