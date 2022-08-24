#pragma once

#include "Model/Import.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace AK::WwiseTransfer
{
	class HierarchyMappingTable
		: public juce::TableListBox
		, public juce::ValueTree::Listener
	{
	public:
		HierarchyMappingTable(juce::ValueTree appState);
		~HierarchyMappingTable() override;

	private:
		struct HierarchyMappingTableModel
			: public juce::TableListBoxModel
		{
			HierarchyMappingTableModel(juce::ValueTree appState);

			int getNumRows() override;
			void paintRowBackground(juce::Graphics&, int rowNumber, int width, int height, bool rowIsSelected) override;
			void paintCell(juce::Graphics&, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
			juce::String getCellTooltip(int rowNumber, int columnId) override;

		private:
			juce::ValueTree hierarchyMapping;
			juce::CachedValue<int> selectedRow;
			juce::CachedValue<bool> applyTemplateFeatureEnabled;
		};

		HierarchyMappingTableModel model;
		juce::ValueTree applicationState;
		juce::ValueTree hierarchyMapping;
		juce::CachedValue<int> selectedRow;
		juce::CachedValue<bool> applyTemplateFeatureEnabled;

		void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
		void valueTreeChildAdded(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded) override;
		void valueTreeChildRemoved(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved) override;
		void valueTreeChildOrderChanged(juce::ValueTree& parentTreeWhoseChildrenHaveMoved, int oldIndex, int newIndex) override;

		void selectedRowsChanged(int row) override;

	private:
		void refreshHeader();

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HierarchyMappingTable)
	};
} // namespace AK::WwiseTransfer
