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

#include <functional>
#include <juce_gui_basics/juce_gui_basics.h>

namespace AK::WwiseTransfer
{
	class HorizontalResizeBar : public juce::Component
	{
	public:
		std::function<void()> onDragStart;
		std::function<void(int)> onDrag;

		HorizontalResizeBar() = default;

		void paint(juce::Graphics& g) override
		{
			auto colour = getLookAndFeel().findColour(juce::GroupComponent::outlineColourId);
			if(isMouseOver())
				colour = colour.brighter(0.5f);
			g.setColour(colour);
			g.fillRect(getLocalBounds().withSizeKeepingCentre(getWidth(), 2));
		}

		void mouseEnter(const juce::MouseEvent&) override
		{
			setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
			repaint();
		}

		void mouseExit(const juce::MouseEvent&) override
		{
			setMouseCursor(juce::MouseCursor::NormalCursor);
			repaint();
		}

		void mouseDown(const juce::MouseEvent& e) override
		{
			dragStartScreenY = e.getScreenY();
			if(onDragStart)
				onDragStart();
		}

		void mouseDrag(const juce::MouseEvent& e) override
		{
			if(onDrag)
				onDrag(e.getScreenY() - dragStartScreenY);
		}

	private:
		int dragStartScreenY = 0;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HorizontalResizeBar)
	};
} // namespace AK::WwiseTransfer
