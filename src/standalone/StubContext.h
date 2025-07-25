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

#include "Core/DawContext.h"

#include <vector>

namespace AK::WwiseTransfer
{
	class StubContext
		: public DawContext
	{
	public:
		bool sessionChanged() override
		{
			return false;
		}

		std::vector<Import::PreviewItem> getItemsForPreview(const Import::Options& options) override
		{
			return std::vector<Import::PreviewItem>{{"\\A\\B\\C", juce::String(), juce::String()}};
		}

		std::vector<Import::Item> getItemsForImport(const Import::Options& options) override
		{
			return std::vector<Import::Item>{{"\\A\\B\\C", juce::String(), juce::String(), juce::String()}};
		}

		void renderItems() override
		{
		}

		// Inherited via DawContext
		juce::String getSessionName() override
		{
			return juce::String();
		}

		bool saveState(juce::ValueTree applicationState) override
		{
			return true;
		}

		juce::ValueTree retrieveState() override
		{
			return juce::ValueTree();
		}
	};
} // namespace AK::WwiseTransfer
