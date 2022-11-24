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

#include <juce_gui_basics/juce_gui_basics.h>

namespace AK::WwiseTransfer
{
	class SimpleListBoxModel
		: public juce::ListBoxModel
	{
	public:
		SimpleListBoxModel(const std::vector<juce::String>& items);

		int getNumRows() override;
		void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
		void listBoxItemClicked(int row, const juce::MouseEvent&) override;
		juce::Value& getSelectedRowValue();

	private:
		const std::vector<juce::String>& items;
		juce::Value selectedRowValue;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleListBoxModel);
	};

	class SimpleListBox
		: public juce::ListBox
		, public juce::Value::Listener
	{
	public:
		SimpleListBox(const std::vector<juce::String>& items);
		~SimpleListBox();

		juce::Value& getSelectedRowValue();
		void valueChanged(juce::Value& value) override;

	private:
		SimpleListBoxModel model;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleListBox);
	};
} // namespace AK::WwiseTransfer
