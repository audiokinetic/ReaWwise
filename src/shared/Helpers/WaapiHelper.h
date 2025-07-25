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

#include "Model/Waapi.h"

#include <JSONHelpers.h>
#include <juce_core/juce_core.h>

namespace AK::WwiseTransfer::WaapiHelper
{
	template <class Function>
	inline void executeWithRetry(Function function, int retryDelayMs = 500, int maxAttempts = 20)
	{
		for(int attempt{}; attempt < maxAttempts && !function(); ++attempt)
			juce::Time::waitForMillisecondCounter(juce::Time::getMillisecondCounter() + retryDelayMs);
	}

	inline Waapi::Error parseError(const juce::String& procedureUri, AK::WwiseAuthoringAPI::AkJson result)
	{
		Waapi::Error error;
		error.procedureUri = procedureUri;
		error.raw = WwiseAuthoringAPI::JSONHelpers::GetAkJsonString(result);

		if(result.HasKey("message"))
			error.message = result["message"].GetVariant().GetString().c_str();

		if(result.HasKey("uri"))
			error.uri = result["uri"].GetVariant().GetString().c_str();

		return error;
	}
} // namespace AK::WwiseTransfer::WaapiHelper
