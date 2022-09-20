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
