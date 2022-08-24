#include "OutputLogComponent.h"

namespace AK::WwiseTransfer
{
	OutputLogComponent::OutputLogComponent()
	{
		setReadOnly(true);
		setMultiLine(true);
		setScrollbarsShown(true);
		insertTextAtCaret("Log Output:");
	}

	void OutputLogComponent::onLogMessage(const juce::String& logMessage)
	{
		juce::String textToInsert = "\n";
		textToInsert << logMessage;

		auto printMessageToTextBox = [this, textToInsert]
		{
			insertTextAtCaret(textToInsert);
		};

		juce::MessageManager::callAsync(printMessageToTextBox);
	}
} // namespace AK::WwiseTransfer
