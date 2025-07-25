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

#include "HierarchyMappingControls.h"

#include "BinaryData.h"
#include "Helpers/ImportHelper.h"
#include "Model/IDs.h"
#include "Model/Import.h"

namespace AK::WwiseTransfer
{
	namespace HierarchyMappingControlsConstants
	{
		constexpr int buttonSize = 24;
	}

	HierarchyMappingControls::HierarchyMappingControls(juce::ValueTree appState, ApplicationProperties& applicationProperties, const juce::String& applicationName)
		: applicationState(appState)
		, hierarchyMapping(applicationState.getChildWithName(IDs::hierarchyMapping))
		, selectedRow(hierarchyMapping, IDs::selectedRow, nullptr)
		, insertButton("InsertButton", juce::Drawable::createFromImageData(BinaryData::General_SmallAdd_Normal_svg, BinaryData::General_SmallAdd_Normal_svgSize))
		, removeButton("RemoveButton", juce::Drawable::createFromImageData(BinaryData::General_SmallDelete_Normal_svg, BinaryData::General_SmallDelete_Normal_svgSize))
		, moveUpButton("MoveUpButton", juce::Drawable::createFromImageData(BinaryData::General_ListMoveUp_Normal_svg, BinaryData::General_ListMoveUp_Normal_svgSize))
		, moveDownButton("MoveDownButton", juce::Drawable::createFromImageData(BinaryData::General_ListMoveDown_Normal_svg, BinaryData::General_ListMoveDown_Normal_svgSize))
		, presetMenuComponent(applicationState, applicationProperties, applicationName)
	{
		insertButton.setTooltip("Insert");
		removeButton.setTooltip("Remove");
		moveUpButton.setTooltip("Move Up");
		moveDownButton.setTooltip("Move Down");

		insertButton.onClick = [this]
		{
			hierarchyMapping.addChild(ImportHelper::hierarchyMappingNodeToValueTree(Import::HierarchyMappingNode("", Wwise::ObjectType::Unknown)), selectedRow, nullptr);
		};

		removeButton.onClick = [this]
		{
			hierarchyMapping.removeChild(selectedRow, nullptr);
		};

		moveUpButton.onClick = [this]
		{
			hierarchyMapping.moveChild(selectedRow, selectedRow - 1, nullptr);
		};

		moveDownButton.onClick = [this]
		{
			hierarchyMapping.moveChild(selectedRow, selectedRow + 1, nullptr);
		};

		addAndMakeVisible(insertButton);
		addAndMakeVisible(removeButton);
		addAndMakeVisible(moveUpButton);
		addAndMakeVisible(moveDownButton);
		addAndMakeVisible(presetMenuComponent);

		applicationState.addListener(this);

		refreshComponent();
	}

	HierarchyMappingControls::~HierarchyMappingControls()
	{
		applicationState.removeListener(this);
	}

	void HierarchyMappingControls::refreshComponent()
	{
		removeButton.setEnabled(hierarchyMapping.getNumChildren() > 1);
		moveUpButton.setEnabled(selectedRow > 0);
		moveDownButton.setEnabled(selectedRow >= 0 && selectedRow < hierarchyMapping.getNumChildren() - 1);
	}

	void HierarchyMappingControls::resized()
	{
		using namespace HierarchyMappingControlsConstants;

		auto area = getLocalBounds();

		juce::FlexBox buttonSection;
		buttonSection.flexDirection = juce::FlexBox::Direction::column;
		buttonSection.justifyContent = juce::FlexBox::JustifyContent::spaceBetween;

		auto buttonSectionWidth = area.getWidth();
		buttonSection.items.add(juce::FlexItem(buttonSectionWidth, buttonSize, presetMenuComponent));
		buttonSection.items.add(juce::FlexItem(buttonSectionWidth, buttonSize, insertButton));
		buttonSection.items.add(juce::FlexItem(buttonSectionWidth, buttonSize, removeButton));
		buttonSection.items.add(juce::FlexItem(buttonSectionWidth, buttonSize, moveUpButton));
		buttonSection.items.add(juce::FlexItem(buttonSectionWidth, buttonSize, moveDownButton));

		buttonSection.performLayout(area);
	}

	void HierarchyMappingControls::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
	{
		if(treeWhosePropertyHasChanged == hierarchyMapping && property == selectedRow.getPropertyID())
		{
			triggerAsyncUpdate();
		}
	}

	void HierarchyMappingControls::valueTreeChildAdded(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded)
	{
		if(parentTree == hierarchyMapping)
		{
			triggerAsyncUpdate();
		}
	}

	void HierarchyMappingControls::valueTreeChildRemoved(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved)
	{
		if(parentTree == hierarchyMapping)
		{
			triggerAsyncUpdate();
		}
	}

	void HierarchyMappingControls::handleAsyncUpdate()
	{
		refreshComponent();
	}
} // namespace AK::WwiseTransfer
