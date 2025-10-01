/*----------------------------------------------------------------------------------------

Copyright (c) 2026 AUDIOKINETIC Inc.

This file is licensed to use under the license available at:
https://github.com/audiokinetic/ReaWwise/blob/main/License.txt (the "License").
You may not use this file except in compliance with the License.

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.  See the License for the
specific language governing permissions and limitations under the License.

----------------------------------------------------------------------------------------*/

#pragma once

#include "Theme/CustomLookAndFeel.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace AK::WwiseTransfer
{
	class OpenSourceComponent : public juce::Component
	{
	public:
		OpenSourceComponent();
		~OpenSourceComponent() override;

		void resized() override;
		void paint(juce::Graphics& g) override;

	private:
		CustomLookAndFeel lookAndFeel;
		juce::TextEditor text;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpenSourceComponent)
	};
} // namespace AK::WwiseTransfer
