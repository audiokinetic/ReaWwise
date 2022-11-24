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

#pragma once

#include "Model/Import.h"

#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>

namespace AK::WwiseTransfer
{
	class DawContext
	{
	public:
		virtual ~DawContext() = default;

		virtual bool sessionChanged() = 0;
		virtual juce::String getSessionName() = 0;
		virtual bool saveState(juce::ValueTree applicationState) = 0;
		virtual juce::ValueTree retrieveState() = 0;
		virtual void renderItems() = 0;
		virtual std::vector<Import::PreviewItem> getItemsForPreview(const Import::Options& options) = 0;
		virtual std::vector<Import::Item> getItemsForImport(const Import::Options& options) = 0;
	};
} // namespace AK::WwiseTransfer
