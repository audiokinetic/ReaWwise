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

#include "Theme/CustomLookAndFeel.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace AK::WwiseTransfer
{
	class AboutComponent : public juce::Component
	{
	public:
		AboutComponent(const juce::String& applicationName);
		~AboutComponent() override;

		void resized() override;
		void paint(juce::Graphics& g);

	private:
		CustomLookAndFeel lookAndFeel;

		juce::Label titleLabel;
		juce::Label versionLabel;
		juce::Label copyrightLabel;

		juce::String copyRightSymbol;

		juce::HyperlinkButton documentationLink;
		juce::HyperlinkButton releaseNotesLink;
		juce::HyperlinkButton licensingLink;

		juce::TooltipWindow tooltipWindow;

		std::unique_ptr<juce::Drawable> wwiseIcon;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AboutComponent)
	};
} // namespace AK::WwiseTransfer
