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
