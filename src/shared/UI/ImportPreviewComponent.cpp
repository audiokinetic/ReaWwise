#include "ImportPreviewComponent.h"

#include "Helpers/ImportHelper.h"
#include "Helpers/WwiseHelper.h"
#include "Model/IDs.h"

#include <set>
#include <unordered_map>

namespace AK::WwiseTransfer
{
	namespace ImportPreviewComponentConstants
	{
		constexpr int headerHeight = 24;
		constexpr int columnInitialWidth = 10;
		constexpr int columnMinimumWidth = 10;
		constexpr int columnMaximumWidth = 1000;
		const std::initializer_list<juce::String> tableHeaders{"Name", "Object Status", "Originals WAV", "WAV Status"};
		constexpr int defaultColumnPropertyFlags = juce::TableHeaderComponent::ColumnPropertyFlags::visible | juce::TableHeaderComponent::ColumnPropertyFlags::resizable | juce::TableHeaderComponent::ColumnPropertyFlags::sortable;
		constexpr int iconPadding = 2;
		constexpr int disabledFlag = 128;
	} // namespace ImportPreviewComponentConstants

	enum TreeValueItemColumn
	{
		Name = 1,
		ObjectStatus,
		OriginalsWav,
		WavStatus
	};

	ImportPreviewComponent::ImportPreviewComponent(juce::ValueTree appState)
		: applicationState(appState)
		, previewItems(applicationState.getChildWithName(IDs::previewItems))
		, rootItem(header, previewItems)
		, previewLoading(applicationState, IDs::previewLoading, nullptr)
	{
		using namespace ImportPreviewComponentConstants;

		auto featureSupport = appState.getChildWithName(IDs::featureSupport);
		applyTemplateFeatureEnabled.referTo(featureSupport, IDs::applyTemplateFeatureEnabled, nullptr);

		treeView.setDefaultOpenness(true);
		treeView.setRootItem(&rootItem);
		treeView.setRootItemVisible(false);

		for(const auto& tableHeader : tableHeaders)
		{
			auto index = (&tableHeader - tableHeaders.begin());
			header.addColumn(tableHeader, index + 1, columnInitialWidth, columnMinimumWidth, columnMaximumWidth, defaultColumnPropertyFlags);
		}

		header.setStretchToFitActive(true);

		applicationState.addListener(this);

		emptyState.setText("Current Render settings will not result in any rendered files.", juce::dontSendNotification);
		emptyState.setJustificationType(juce::Justification::centred);

		addAndMakeVisible(header);
		addAndMakeVisible(treeView);
		addChildComponent(loadingComponent);

		refreshHeader();

		addChildComponent(emptyState);
	}

	ImportPreviewComponent::~ImportPreviewComponent()
	{
		applicationState.removeListener(this);
	}

	void ImportPreviewComponent::resized()
	{
		auto area = getLocalBounds();

		header.setBounds(area.removeFromTop(ImportPreviewComponentConstants::headerHeight));
		header.resizeAllColumnsToFit(area.getWidth());

		treeView.setBounds(area);

		loadingComponent.setBounds(area);

		emptyState.setBounds(area);
	}

	void ImportPreviewComponent::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged,
		const juce::Identifier& property)
	{
		if(property == IDs::previewLoading)
		{
			triggerAsyncUpdate();
		}

		if(treeWhosePropertyHasChanged.getType() == IDs::featureSupport && property == IDs::applyTemplateFeatureEnabled)
		{
			refreshHeader();
		}
	}

	void ImportPreviewComponent::valueTreeChildAdded(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded)
	{
		if(parentTree.getType() == IDs::previewItems)
		{
			triggerAsyncUpdate();
		}
	}

	void ImportPreviewComponent::valueTreeChildRemoved(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved)
	{
		if(parentTree.getType() == IDs::previewItems)
		{
			triggerAsyncUpdate();
		}
	}

	void ImportPreviewComponent::handleAsyncUpdate()
	{
		loadingComponent.setVisible(previewLoading);
		emptyState.setVisible(previewItems.getNumChildren() == 0 && !previewLoading);
	}

	void ImportPreviewComponent::refreshHeader()
	{
		using namespace ImportPreviewComponentConstants;

		// For some reason, the column width measurement only works properly if the following attribute is set to false
		header.setStretchToFitActive(false);

		auto currentWidth = header.getColumnWidth(tableHeaders.size());
		auto propertyFlags = defaultColumnPropertyFlags;

		if(!applyTemplateFeatureEnabled.get())
			propertyFlags |= disabledFlag;

		header.removeColumn(tableHeaders.size());
		header.addColumn(*(tableHeaders.end() - 1), tableHeaders.size(), currentWidth, columnMinimumWidth, columnMaximumWidth, propertyFlags);

		header.setStretchToFitActive(true);
	}

	ValueTreeItem::ValueTreeItem(juce::TableHeaderComponent& header, juce::ValueTree t)
		: header(header)
		, tree(t)
	{
		tree.addListener(this);
		header.addListener(this);
	}

	ValueTreeItem::~ValueTreeItem()
	{
		tree.removeListener(this);
		header.removeListener(this);
	}

	bool ValueTreeItem::mightContainSubItems()
	{
		return tree.getNumChildren() > 0;
	}

	void ValueTreeItem::itemOpennessChanged(bool isNowOpen)
	{
		if(isNowOpen && getNumSubItems() == 0)
		{
			refreshSubItems();
		}
		else
		{
			// TODO: there may be a performance benefit to call clear subitems. The issue with calling clearSubItems()
			// here is that it loses the "openness" state of the tree.
			// clearSubItems();
		}
	}

	void ValueTreeItem::refreshSubItems()
	{
		clearSubItems();

		auto isSorted = header.getSortColumnId() > 0;

		if(isSorted)
		{
			auto comparator = Comparator(header);

			for(int i = 0; i < tree.getNumChildren(); ++i)
			{
				addSubItemSorted(comparator, new ValueTreeItem(header, tree.getChild(i)));
			}
		}
		else
		{
			for(int i = 0; i < tree.getNumChildren(); ++i)
			{
				addSubItem(new ValueTreeItem(header, tree.getChild(i)));
			}
		}
	}

	void ValueTreeItem::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&)
	{
		repaintItem();
	}

	void ValueTreeItem::valueTreeChildAdded(juce::ValueTree& parentTree, juce::ValueTree&)
	{
		treeChildrenChanged(parentTree);
	}

	void ValueTreeItem::valueTreeChildRemoved(juce::ValueTree& parentTree, juce::ValueTree&, int)
	{
		treeChildrenChanged(parentTree);
	}

	void ValueTreeItem::valueTreeChildOrderChanged(juce::ValueTree& parentTree, int, int)
	{
		treeChildrenChanged(parentTree);
	}

	void ValueTreeItem::valueTreeParentChanged(juce::ValueTree&)
	{
	}

	void ValueTreeItem::treeChildrenChanged(const juce::ValueTree& parentTree)
	{
		if(parentTree == tree)
		{
			refreshSubItems();
			treeHasChanged();
		}
	}

	void ValueTreeItem::paintItem(juce::Graphics& g, int width, int height)
	{
		using namespace ImportPreviewComponentConstants;

		auto previewItem = ImportHelper::valueTreeToPreviewItem(tree);

		std::array<juce::String, 4> cellText{
			previewItem.name,
			ImportHelper::objectStatusToReadableString(previewItem.objectStatus),
			previewItem.audioFilePath,
			ImportHelper::wavStatusToReadableString(previewItem.wavStatus),
		};

		jassert(cellText.size() == header.getNumColumns(true));

		auto* customLookAndFeel = dynamic_cast<CustomLookAndFeel*>(&getOwnerView()->getLookAndFeel());
		g.setColour(customLookAndFeel->getTextColourForObjectStatus(previewItem.objectStatus));

		// First column is special since it has indentation + icon
		auto indent = getItemPosition(true).getX();
		int iconSize = height;
		int trueColumnWidth = header.getColumnWidth(1) - indent;
		int textWidth = trueColumnWidth - iconSize;

		auto icon = customLookAndFeel->getIconForObjectType(previewItem.type);

		g.setFont(CustomLookAndFeelConstants::smallFontSize);

		if(trueColumnWidth > 0)
		{
			icon->drawWithin(g, juce::Rectangle<float>(iconSize, iconSize).reduced(iconPadding, iconPadding), juce::RectanglePlacement::centred, 1);

			if(textWidth > 0)
			{
				g.drawText(cellText[0],
					iconSize, 0, textWidth, height,
					juce::Justification::centredLeft, true);
			}
		}

		auto xPosition = trueColumnWidth;

		// The rest of the cells are just text, so no special text coordinates to calculate
		for(int i = 1; i < cellText.size(); ++i)
		{
			auto columnWidth = header.getColumnWidth(i + 1);

			g.drawText(cellText[i],
				xPosition, 0, columnWidth, height,
				juce::Justification::centredLeft, true);

			xPosition += columnWidth;
		}
	}

	void ValueTreeItem::tableColumnsChanged(juce::TableHeaderComponent* tableHeader)
	{
	}

	void ValueTreeItem::tableColumnsResized(juce::TableHeaderComponent* tableHeader)
	{
		repaintItem();
	}

	void ValueTreeItem::tableSortOrderChanged(juce::TableHeaderComponent* tableHeader)
	{
		if(isOpen())
		{
			auto comparator = Comparator(header);

			sortSubItems(comparator);
			treeHasChanged();
		}
	}

	juce::String ValueTreeItem::getUniqueName() const
	{
		return tree.getType().toString();
	}

	juce::String ValueTreeItem::getComparisonTextForColumn(int column)
	{
		auto previewItem = ImportHelper::valueTreeToPreviewItem(tree);

		switch(column)
		{
		case TreeValueItemColumn::Name:
			return getUniqueName();
		case TreeValueItemColumn::ObjectStatus:
			return ImportHelper::objectStatusToReadableString(previewItem.objectStatus);
		case TreeValueItemColumn::OriginalsWav:
			return previewItem.audioFilePath;
		case TreeValueItemColumn::WavStatus:
			// TODO: Return the wav status as string eventually
			return juce::String();
		default:
			return juce::String();
		}
	}

	ValueTreeItem::Comparator::Comparator(const juce::TableHeaderComponent& header)
		: sortColumnId(header.getSortColumnId() == 0 ? 1 : header.getSortColumnId())
		, sortDirectionForward(header.getSortColumnId() == 0 ? true : header.isSortedForwards())
	{
	}

	int ValueTreeItem::Comparator::compareElements(juce::TreeViewItem* first, juce::TreeViewItem* second)
	{
		auto firstAsValueTreeItem = dynamic_cast<ValueTreeItem*>(first);
		auto secondAsValueTreeItem = dynamic_cast<ValueTreeItem*>(second);

		if(firstAsValueTreeItem && secondAsValueTreeItem)
		{
			auto firstTextValue = firstAsValueTreeItem->getComparisonTextForColumn(sortColumnId);
			auto secondTextValue = secondAsValueTreeItem->getComparisonTextForColumn(sortColumnId);

			auto compareVal = firstTextValue.compareNatural(secondTextValue);

			return sortDirectionForward ? compareVal : -1 * compareVal;
		}

		return 0;
	}
} // namespace AK::WwiseTransfer
