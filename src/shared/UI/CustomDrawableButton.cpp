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

#include "CustomDrawableButton.h"

namespace AK::WwiseTransfer
{
	namespace CustomDrawableButtonConstants
	{
		constexpr int margin = 2;
	}

	CustomDrawableButton::CustomDrawableButton(const juce::String& buttonName, std::unique_ptr<juce::Drawable> drawableValue)
		: juce::DrawableButton(buttonName, juce::DrawableButton::ButtonStyle::ImageOnButtonBackground)
		, drawable(std::move(drawableValue))
	{
		setImages(drawable.get());
	}

	juce::Rectangle<float> CustomDrawableButton::getImageBounds() const
	{
		return getLocalBounds().reduced(CustomDrawableButtonConstants::margin).toFloat();
	}
} // namespace AK::WwiseTransfer
