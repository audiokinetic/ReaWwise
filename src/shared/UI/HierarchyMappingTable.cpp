#include "HierarchyMappingTable.h"

#include "Helpers/ImportHelper.h"
#include "Model/Import.h"
#include "Theme/CustomLookAndFeel.h"

namespace AK::WwiseTransfer
{
	enum HierarchyMappingTableColumn
	{
		ObjectType = 1,
		ObjectName,
		PropertyTemplatePath
	};

	namespace HierarchyMappingTableConstants
	{
		constexpr int tableHeaderHeight = 24;
		constexpr int columnInitialWidth = 10;
		constexpr int margin = 4;
		inline const std::initializer_list<juce::String> tableHeaders{"Object Type", "Object Name", "Property Template Path"};
		constexpr int columnMinimumWidth = 10;
		constexpr int columnMaximumWidth = 1000;
		constexpr int defaultColumnPropertyFlags = juce::TableHeaderComponent::ColumnPropertyFlags::visible | juce::TableHeaderComponent::ColumnPropertyFlags::resizable;
		constexpr int disabledFlag = 128;
	} // namespace HierarchyMappingTableConstants

	HierarchyMappingTable::HierarchyMappingTable(juce::ValueTree appState)
		: applicationState(appState)
		, hierarchyMapping(applicationState.getChildWithName(IDs::hierarchyMapping))
		, selectedRow(hierarchyMapping, IDs::selectedRow, nullptr)
		, model(appState)
	{
		using namespace HierarchyMappingTableConstants;

		auto featureSupport = appState.getChildWithName(IDs::featureSupport);
		applyTemplateFeatureEnabled.referTo(featureSupport, IDs::applyTemplateFeatureEnabled, nullptr);

		setModel(&model);

		for(const auto& tableHeader : tableHeaders)
		{
			auto index = (&tableHeader - tableHeaders.begin());
			getHeader().addColumn(tableHeader, index + 1, columnInitialWidth, columnMinimumWidth, columnMaximumWidth, defaultColumnPropertyFlags);
		}

		getHeader().setPopupMenuActive(false);
		setHeaderHeight(tableHeaderHeight);
		applicationState.addListener(this);

		selectRow(selectedRow.get());

		refreshHeader();
	}

	HierarchyMappingTable::~HierarchyMappingTable()
	{
		applicationState.removeListener(this);
	}

	void HierarchyMappingTable::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
	{
		auto treeType = treeWhosePropertyHasChanged.getType();

		if(treeType == IDs::hierarchyMappingNode || treeType == IDs::hierarchyMapping && property == IDs::selectedRow)
		{
			repaint();
		}

		if(treeType == IDs::featureSupport && property == IDs::applyTemplateFeatureEnabled)
		{
			refreshHeader();
		}
	}

	void HierarchyMappingTable::valueTreeChildAdded(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded)
	{
		if(parentTree.getType() == IDs::hierarchyMapping)
		{
			updateContent();
			resized();

			if(parentTree.getNumChildren() == 1)
				selectRow(0);
		}
	}

	void HierarchyMappingTable::valueTreeChildRemoved(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved)
	{
		if(parentTree.getType() == IDs::hierarchyMapping)
		{
			updateContent();
			resized();

			if(selectedRow == -1 && indexFromWhichChildWasRemoved != 0)
				selectRow(indexFromWhichChildWasRemoved - 1);
		}
	}

	void HierarchyMappingTable::valueTreeChildOrderChanged(juce::ValueTree& parentTreeWhoseChildrenHaveMoved, int oldIndex, int newIndex)
	{
		if(parentTreeWhoseChildrenHaveMoved.getType() == IDs::hierarchyMapping)
		{
			repaint();

			selectRow(newIndex);
		}
	}

	void HierarchyMappingTable::selectedRowsChanged(int row)
	{
		selectedRow = row;
	}

	void HierarchyMappingTable::refreshHeader()
	{
		using namespace HierarchyMappingTableConstants;

		// For some reason, the column width measurement only works properly if the following attribute is set to false
		getHeader().setStretchToFitActive(false);

		auto currentWidth = getHeader().getColumnWidth(tableHeaders.size());
		auto propertyFlags = defaultColumnPropertyFlags;

		if(!applyTemplateFeatureEnabled.get())
			propertyFlags |= disabledFlag;

		getHeader().removeColumn(tableHeaders.size());
		getHeader().addColumn(*(tableHeaders.end() - 1), tableHeaders.size(), currentWidth, columnMinimumWidth, columnMaximumWidth, propertyFlags);

		getHeader().setStretchToFitActive(true);
	}

	HierarchyMappingTable::HierarchyMappingTableModel::HierarchyMappingTableModel(juce::ValueTree appState)
		: hierarchyMapping(appState.getChildWithName(IDs::hierarchyMapping))
	{
		auto featureSupport = appState.getChildWithName(IDs::featureSupport);
		applyTemplateFeatureEnabled.referTo(featureSupport, IDs::applyTemplateFeatureEnabled, nullptr);
	}

	int HierarchyMappingTable::HierarchyMappingTableModel::getNumRows()
	{
		return hierarchyMapping.getNumChildren();
	}

	void HierarchyMappingTable::HierarchyMappingTableModel::paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
	{
		if(rowIsSelected)
			g.fillAll(juce::LookAndFeel::getDefaultLookAndFeel().findColour(juce::TextEditor::highlightColourId));
		else
			g.fillAll(juce::LookAndFeel::getDefaultLookAndFeel().findColour(juce::ListBox::backgroundColourId));
	}

	void HierarchyMappingTable::HierarchyMappingTableModel::paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
	{
		using namespace HierarchyMappingTableConstants;

		auto hierarchyMappingNode = ImportHelper::valueTreeToHiarchyMappingNode(hierarchyMapping.getChild(rowNumber));

		juce::String text;
		bool cellEnabled = true;
		bool validCell = true;

		switch(columnId)
		{
		case HierarchyMappingTableColumn::ObjectType:
		{
			text = hierarchyMappingNode.type == Wwise::ObjectType::Unknown ? "Select Object Type" : WwiseHelper::objectTypeToReadableString(hierarchyMappingNode.type);
			validCell = hierarchyMappingNode.typeValid;
			break;
		}
		case HierarchyMappingTableColumn::ObjectName:
		{
			text = hierarchyMappingNode.name;
			validCell = hierarchyMappingNode.nameValid;
			break;
		}
		case HierarchyMappingTableColumn::PropertyTemplatePath:
		{
			text = hierarchyMappingNode.propertyTemplatePath;
			cellEnabled = hierarchyMappingNode.propertyTemplatePathEnabled && applyTemplateFeatureEnabled;
			validCell = hierarchyMappingNode.propertyTemplatePathValid;
			break;
		}
		}

		// Cell color depends on table enablement, field enablement and field error STATE
		auto textColor = juce::LookAndFeel::getDefaultLookAndFeel().findColour(juce::TextEditor::textColourId);

		auto alpha = cellEnabled ? 1.0f : 0.5f;

		g.setFont(CustomLookAndFeelConstants::smallFontSize);
		g.setColour(textColor.withAlpha(alpha));
		g.drawText(text, margin, 0, width, height, juce::Justification::left);

		if(cellEnabled && !validCell)
		{
			g.setColour(juce::Colour(juce::Colours::red).withAlpha(alpha));
			g.drawRect(0, 0, width, height);
		}
	}

	juce::String HierarchyMappingTable::HierarchyMappingTableModel::getCellTooltip(int rowNumber, int columnId)
	{
		juce::String tooltipText;

		auto hierarchyMappingNode = ImportHelper::valueTreeToHiarchyMappingNode(hierarchyMapping.getChild(rowNumber));

		switch(columnId)
		{
		case HierarchyMappingTableColumn::ObjectType:
		{
			tooltipText = hierarchyMappingNode.typeErrorMessage;
			break;
		}
		case HierarchyMappingTableColumn::ObjectName:
		{
			tooltipText = hierarchyMappingNode.nameErrorMessage;
			break;
		}
		case HierarchyMappingTableColumn::PropertyTemplatePath:
		{
			tooltipText = hierarchyMappingNode.propertyTemplatePathErrorMessage;
			break;
		}
		}

		return tooltipText;
	}
} // namespace AK::WwiseTransfer
