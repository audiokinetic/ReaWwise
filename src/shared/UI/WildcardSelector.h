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

#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>

namespace AK::WwiseTransfer
{
	class WildcardSelector
		: public juce::TextButton
	{
	public:
		WildcardSelector();

		std::function<void(const juce::String&)> onItemSelected;

	private:
		void showWildcardMenu();

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WildcardSelector);
	};
} // namespace AK::WwiseTransfer
