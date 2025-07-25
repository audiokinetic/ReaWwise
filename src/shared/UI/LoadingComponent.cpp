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

#include "LoadingComponent.h"

namespace AK::WwiseTransfer
{
	namespace LoadingComponentConstants
	{
		constexpr int loaderSize = 25;
		constexpr int textHeight = 24;
		constexpr float backgroundOpacity = 0.8f;
	} // namespace LoadingComponentConstants

	LoadingComponent::LoadingComponent()
		: progressBar(progress)
	{
		text.setText("Loading preview ...", juce::dontSendNotification);
		text.setJustificationType(juce::Justification::centred);

		addAndMakeVisible(progressBar);
		addAndMakeVisible(text);
	}

	void LoadingComponent::resized()
	{
		using namespace LoadingComponentConstants;

		auto area = getLocalBounds();

		juce::FlexBox fb;
		fb.flexDirection = juce::FlexBox::Direction::column;
		fb.alignItems = juce::FlexBox::AlignItems::center;
		fb.justifyContent = juce::FlexBox::JustifyContent::center;

		fb.items.add(juce::FlexItem(progressBar).withWidth(loaderSize).withHeight(loaderSize));
		fb.items.add(juce::FlexItem(text).withWidth(area.getWidth()).withHeight(textHeight));

		fb.performLayout(area);
	}

	void LoadingComponent::paint(juce::Graphics& g)
	{
		auto color = getLookAndFeel()
		                 .findColour(juce::ResizableWindow::ColourIds::backgroundColourId)
		                 .withAlpha(LoadingComponentConstants::backgroundOpacity);

		g.fillAll(color);
	}
} // namespace AK::WwiseTransfer
