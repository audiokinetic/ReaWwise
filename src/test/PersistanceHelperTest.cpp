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

#include "PersistanceHelperTest.h"


namespace AK::WwiseTransfer::Test
{
	TEST_CASE("hierarchyMappingToPresetData")
	{
		auto childrenCount = 3;

		auto rootValueTree = juce::ValueTree(IDs::hierarchyMapping);
		auto mappingNodeValuesVector = std::vector<TestHierarchyMappingNodeValues>();

		for (int index = 1; index <= childrenCount; index++)
		{
			auto mappingNodeValues = TestHierarchyMappingNodeValues(index);
			mappingNodeValuesVector.emplace_back(mappingNodeValues);

			auto childValueTree = mappingNodeValues.generateValueTree();
			rootValueTree.appendChild(childValueTree, nullptr);
		}

		SECTION("Without Removed Properties")
		{
			auto presetData = PersistanceHelper::hierarchyMappingToPresetData(rootValueTree);
			auto parsedData = juce::parseXML(presetData);

			for (int index = 0; index < childrenCount; index++)
			{
				auto childPreset = parsedData.get()->getChildElement(index);
				testHierarchyMappingPresetDataEquality(*childPreset, mappingNodeValuesVector[index]);
			}
		}

		SECTION("With Removed Properties")
		{
			for (int index = 0; index < childrenCount; index++)
			{
				auto childTree = rootValueTree.getChild(index);
				addPropertiesToRemove(childTree);
			}

			auto presetData = PersistanceHelper::hierarchyMappingToPresetData(rootValueTree);
			auto parsedData = juce::parseXML(presetData);

			for (int index = 0; index < childrenCount; index++)
			{
				auto childPreset = parsedData.get()->getChildElement(index);
				testHierarchyMappingPresetDataEquality(*childPreset, mappingNodeValuesVector[index]);
				testHierarchyMappingPresetRemovedProperties(*childPreset);
			}
		}
	}

	TEST_CASE("hierarchyMappingToPresetData: Empty Value Tree")
	{
		auto valueTree = juce::ValueTree();
		auto presetData = PersistanceHelper::hierarchyMappingToPresetData(valueTree);

		REQUIRE(presetData.isEmpty());
	}

	TEST_CASE("presetDataToHierarchyMapping")
	{
		auto rootValueTree = juce::ValueTree(IDs::hierarchyMapping);

		auto childMappingValues = TestHierarchyMappingNodeValues();
		auto childValueTree = childMappingValues.generateValueTree();

		rootValueTree.addChild(childValueTree, 0, nullptr);
		auto rootPresetData = rootValueTree.toXmlString();
		auto valueTree = PersistanceHelper::presetDataToHierarchyMapping(rootPresetData);

		for (int index = 0; index < valueTree.getNumChildren(); index++)
		{
			testHierarchyMappingValueTreeEquality(valueTree.getChild(index), childMappingValues);
		}
	}

	TEST_CASE("presetDataToHierarchyMapping: Empty String")
	{
		auto hierarchyMapping = PersistanceHelper::presetDataToHierarchyMapping("");

		REQUIRE_FALSE(hierarchyMapping.isValid());
	}
} // namespace AK::WwiseTransfer::Test
