#pragma once

#include "AK/WwiseAuthoringAPI/AkAutobahn/AkJson.h"

#include <juce_core/juce_core.h>

namespace AK::WwiseTransfer::WaapiHelper
{
	template <class Function>
	inline void executeWithRetry(Function function, int retryDelayMs = 500, int maxAttempts = 20)
	{
		for(int attempt{}; attempt < maxAttempts && !function(); ++attempt)
			juce::Time::waitForMillisecondCounter(juce::Time::getMillisecondCounter() + retryDelayMs);
	}

	inline juce::String getErrorMessage(AK::WwiseAuthoringAPI::AkJson result)
	{
		juce::String message;

		if(result.HasKey("message"))
		{
			message << "Error: ";
			message << result["message"].GetVariant().GetString().c_str();

			if(result.HasKey("details") && result["details"].HasKey("log"))
			{
				message << juce::NewLine() << "Details:" << juce::NewLine() << juce::NewLine();
				for(auto& item : result["details"]["log"].GetArray())
				{
					if(item.HasKey("message"))
						message << juce::NewLine() << item["message"].GetVariant().GetString();
				}
			}
		}

		return message;
	}
} // namespace AK::WwiseTransfer::WaapiHelper
