#pragma once

#include "BinaryData.h"
#include "Core/DawContext.h"
#include "Helpers/ImportHelper.h"
#include "LoadingComponent.h"
#include "Theme/CustomLookAndFeel.h"

#include <juce_gui_basics/juce_gui_basics.h>
#include <unordered_map>

namespace AK::WwiseTransfer
{
	struct ValueTreeItem
		: public juce::TreeViewItem
		, public juce::ValueTree::Listener
		, public juce::TableHeaderComponent::Listener
	{
		struct Comparator
		{
			Comparator(const juce::TableHeaderComponent& header);

			int compareElements(juce::TreeViewItem* first, juce::TreeViewItem* second);

		private:
			const int sortColumnId;
			const bool sortDirectionForward;
		};

		ValueTreeItem(juce::TableHeaderComponent& header, juce::ValueTree tree);
		~ValueTreeItem();

		bool mightContainSubItems() override;
		void itemOpennessChanged(bool isNowOpen) override;
		juce::String getUniqueName() const override;
		juce::String getComparisonTextForColumn(int column);

	private:
		juce::TableHeaderComponent& header;
		juce::ValueTree tree;

		void refreshSubItems();
		void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) override;
		void valueTreeChildAdded(juce::ValueTree& parentTree, juce::ValueTree&) override;
		void valueTreeChildRemoved(juce::ValueTree& parentTree, juce::ValueTree&, int) override;
		void valueTreeChildOrderChanged(juce::ValueTree& parentTree, int, int) override;
		void valueTreeParentChanged(juce::ValueTree&) override;
		void treeChildrenChanged(const juce::ValueTree& parentTree);
		void paintItem(juce::Graphics& g, int width, int height) override;
		void tableColumnsChanged(juce::TableHeaderComponent* tableHeader) override;
		void tableColumnsResized(juce::TableHeaderComponent* tableHeader) override;
		void tableSortOrderChanged(juce::TableHeaderComponent* tableHeader) override;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ValueTreeItem)
	};

	class ImportPreviewComponent
		: public juce::Component
		, public juce::ValueTree::Listener
		, public juce::AsyncUpdater
	{
	public:
		ImportPreviewComponent(juce::ValueTree appState);
		~ImportPreviewComponent();

		void resized() override;

	private:
		juce::ValueTree applicationState;
		juce::ValueTree previewItems;
		juce::TableHeaderComponent header;
		juce::TreeView treeView;
		ValueTreeItem rootItem;

		juce::CachedValue<bool> previewLoading;
		juce::CachedValue<bool> applyTemplateFeatureEnabled;

		LoadingComponent loadingComponent;

		juce::Label emptyState;

		void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
		void valueTreeChildAdded(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded);
		void valueTreeChildRemoved(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved);

		// Inherited via AsyncUpdater
		void handleAsyncUpdate() override;
		void refreshHeader();

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ImportPreviewComponent)
	};
} // namespace AK::WwiseTransfer
