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

#include "Splitter.h"

namespace AK::WwiseTransfer
{
	namespace SplitterConstants
	{
		constexpr int padding = 2;
		constexpr int lineHeight = 2;
		constexpr int lineCenterOffset = 1;
	} // namespace SplitterConstants

	void Splitter::paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
	{
		using namespace SplitterConstants;

		auto area = getLocalBounds();
		auto triangleArea = area.removeFromRight(area.getHeight());

		g.setColour(getLookAndFeel().findColour(juce::GroupComponent::outlineColourId));

		juce::Path path;
		if(getToggleState())
			path.addTriangle(triangleArea.getCentre().withX(triangleArea.getX()).toFloat(), triangleArea.getTopRight().toFloat(), triangleArea.getBottomRight().toFloat());
		else
			path.addTriangle(triangleArea.getBottomLeft().toFloat(), triangleArea.getCentre().withY(triangleArea.getY()).toFloat(), triangleArea.getBottomRight().toFloat());

		g.fillPath(path);

		auto lineRect = juce::Rectangle(area.getX(), area.getCentreY() - lineCenterOffset, area.getWidth() - padding, lineHeight);

		g.fillRect(lineRect);
	}
} // namespace AK::WwiseTransfer
