#include "SimpleListBox.h"

namespace AK::WwiseTransfer
{
	SimpleListBoxModel::SimpleListBoxModel(const std::vector<juce::String>& items)
		: items(items)
		, selectedRowValue(juce::var(0))
	{
	}

	int SimpleListBoxModel::getNumRows()
	{
		return items.size();
	}

	void SimpleListBoxModel::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected)
	{
		if(rowIsSelected)
			g.fillAll(juce::LookAndFeel::getDefaultLookAndFeel().findColour(juce::TextEditor::highlightColourId));

		g.setColour(juce::Colours::white);
		g.setFont(height * 0.7f);

		if(rowNumber < items.size())
		{
			g.drawText(items.at(rowNumber), 5, 0, width, height,
				juce::Justification::centredLeft, true);
		}
	}

	void SimpleListBoxModel::listBoxItemClicked(int row, const juce::MouseEvent&)
	{
		selectedRowValue = row;
	}

	juce::Value& SimpleListBoxModel::getSelectedRowValue()
	{
		return selectedRowValue;
	}

	SimpleListBox::SimpleListBox(const std::vector<juce::String>& items)
		: model(items)
	{
		model.getSelectedRowValue().addListener(this);

		setModel(&model);
	}

	SimpleListBox::~SimpleListBox()
	{
		model.getSelectedRowValue().removeListener(this);
	}

	juce::Value& SimpleListBox::getSelectedRowValue()
	{
		return model.getSelectedRowValue();
	}

	void SimpleListBox::valueChanged(juce::Value& value)
	{
		auto& selectedRowValue = model.getSelectedRowValue();

		if(value.refersToSameSourceAs(selectedRowValue) && selectedRowValue != getSelectedRow())
			selectRow(selectedRowValue.getValue());
	}
} // namespace AK::WwiseTransfer
