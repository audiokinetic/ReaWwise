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
