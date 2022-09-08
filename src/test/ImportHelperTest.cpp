#include "ImportHelperTest.h"

namespace AK::WwiseTransfer::Test
{
	TEST_CASE("hierarchyMappingNodeToValueTree")
	{
		auto testValues = TestTreeHierarchyValues();
		auto hierarchyMappingNode = testValues.generateHierarchyMappingNode();

		auto valueTree = ImportHelper::hierarchyMappingNodeToValueTree(hierarchyMappingNode);
		testValueTreeEquality(valueTree, testValues);
	}

	TEST_CASE("hierachyMappingNodeListToValueTree")
	{
		auto indexLimit = GENERATE(1, 3);
		auto nodes = std::vector<Import::HierarchyMappingNode>();
		auto testValuesVector = std::vector<TestTreeHierarchyValues>();

		for(int index = 0; index < indexLimit; index++)
		{
			auto testValues = TestTreeHierarchyValues(index);
			testValuesVector.emplace_back(testValues);
			auto hierarchyMappingNode = testValues.generateHierarchyMappingNode();
			nodes.emplace_back(hierarchyMappingNode);
		}

		auto valueTrees = ImportHelper::hierachyMappingNodeListToValueTree(nodes);

		for(int index = 0; index < indexLimit; index++)
		{
			testValueTreeEquality(valueTrees.getChild(index), testValuesVector[index]);
		}
	}

	TEST_CASE("valueTreeToHiarchyMappingNode")
	{
		auto testValues = TestTreeHierarchyValues();
		auto valueTree = testValues.generateValueTree();

		auto hierarchyMapping = ImportHelper::valueTreeToHiarchyMappingNode(valueTree);
		testHierarchyMappingEquality(hierarchyMapping, testValues);
	}

	TEST_CASE("valueTreeToPreviewItemNode")
	{
		auto testValues = TestTreePreviewItemValues();
		auto valueTree = testValues.generateValueTree();
		auto previewItem = ImportHelper::valueTreeToPreviewItemNode(valueTree);

		REQUIRE(previewItem.audioFilePath == testValues.audioFilePath);
		REQUIRE(previewItem.name == testValues.objectName);
		REQUIRE(previewItem.objectStatus == testValues.objectStatus);
		REQUIRE(previewItem.type == testValues.objectType);
		REQUIRE(previewItem.wavStatus == testValues.wavStatus);
	}

	TEST_CASE("previewItemNodeToValueTree")
	{
		auto testValues = TestTreePreviewItemValues();
		auto previewItem = testValues.generatePreviewItem();
		auto testPath = "\\test\\path";

		auto valueTree = ImportHelper::previewItemNodeToValueTree(testPath, previewItem);

		REQUIRE(valueTree.getType().toString() == testPath);
		REQUIRE(valueTree.getProperty(IDs::objectName) == testValues.objectName);
		REQUIRE(valueTree.getProperty(IDs::audioFilePath) == testValues.audioFilePath);
		REQUIRE(valueTree.getProperty(IDs::objectType) == juce::VariantConverter<Wwise::ObjectType>::toVar(testValues.objectType));
		REQUIRE(valueTree.getProperty(IDs::objectStatus) == juce::VariantConverter<Import::ObjectStatus>::toVar(testValues.objectStatus));
		REQUIRE(valueTree.getProperty(IDs::wavStatus) == juce::VariantConverter<Import::WavStatus>::toVar(testValues.wavStatus));
	}

	TEST_CASE("valueTreeToHierarchyMappingNodeList")
	{
		auto indexLimit = GENERATE(1, 3);
		auto valueTrees = juce::ValueTree("root");
		auto testValuesVector = std::vector<TestTreeHierarchyValues>();

		for(int index = 0; index < indexLimit; index++)
		{
			auto testValues = TestTreeHierarchyValues(index);
			testValuesVector.emplace_back(testValues);
			auto valueTree = testValues.generateValueTree();
			valueTrees.appendChild(valueTree, nullptr);
		}

		auto hierarchyList = ImportHelper::valueTreeToHierarchyMappingNodeList(valueTrees);

		for(int index = 0; index < indexLimit; index++)
		{
			testHierarchyMappingEquality(hierarchyList[index], testValuesVector[index]);
		}
	}

	TEST_CASE("hierarchyMappingToPath")
	{
		auto indexLimit = GENERATE(1, 3);
		auto nodes = std::vector<Import::HierarchyMappingNode>();
		auto testValuesVector = std::vector<TestTreeHierarchyValues>();

		for(int index = 0; index < indexLimit; index++)
		{
			auto testValues = TestTreeHierarchyValues(index);
			testValuesVector.emplace_back(testValues);
			auto hierarchyMappingNode = testValues.generateHierarchyMappingNode();
			nodes.emplace_back(hierarchyMappingNode);
		}

		auto fullPath = ImportHelper::hierarchyMappingToPath(nodes);

		juce::StringArray pathList;
		pathList.addTokens(fullPath, "\\", "");
		pathList.removeEmptyStrings();

		for(int index = 0; index < pathList.size(); index++)
		{
			REQUIRE(pathList[index].contains(testValuesVector[index].objectName));
			REQUIRE(pathList[index].contains(WwiseHelper::objectTypeToReadableString(testValuesVector[index].objectType)));
		}
	}

	TEST_CASE("containerNameExistsOptionToString")
	{
		REQUIRE(ImportHelper::containerNameExistsOptionToString(Import::ContainerNameExistsOption::CreateNew) == "createNew");
		REQUIRE(ImportHelper::containerNameExistsOptionToString(Import::ContainerNameExistsOption::Replace) == "replaceExisting");
		REQUIRE(ImportHelper::containerNameExistsOptionToString(Import::ContainerNameExistsOption::UseExisting) == "useExisting");
		REQUIRE(ImportHelper::containerNameExistsOptionToString(Import::ContainerNameExistsOption::Unknown) == "notImplemented");
	}

	TEST_CASE("stringToContainerNameExistsOption")
	{
		REQUIRE(ImportHelper::stringToContainerNameExistsOption("createNew") == Import::ContainerNameExistsOption::CreateNew);
		REQUIRE(ImportHelper::stringToContainerNameExistsOption("replace") == Import::ContainerNameExistsOption::Replace);
		REQUIRE(ImportHelper::stringToContainerNameExistsOption("useExisting") == Import::ContainerNameExistsOption::UseExisting);
		REQUIRE(ImportHelper::stringToContainerNameExistsOption("notImplemented") == Import::ContainerNameExistsOption::Unknown);
		REQUIRE(ImportHelper::stringToContainerNameExistsOption("asdfasdf") == Import::ContainerNameExistsOption::Unknown);
	}

	TEST_CASE("containerNameExistsOptionToReadableString")
	{
		REQUIRE(ImportHelper::containerNameExistsOptionToReadableString(Import::ContainerNameExistsOption::CreateNew) == "Create New");
		REQUIRE(ImportHelper::containerNameExistsOptionToReadableString(Import::ContainerNameExistsOption::Replace) == "Replace");
		REQUIRE(ImportHelper::containerNameExistsOptionToReadableString(Import::ContainerNameExistsOption::UseExisting) == "Use Existing");
		REQUIRE(ImportHelper::containerNameExistsOptionToReadableString(Import::ContainerNameExistsOption::Unknown) == "Not Implemented");
	}

	TEST_CASE("audioFilenameExistsOptionToReadableString")
	{
		REQUIRE(ImportHelper::audioFilenameExistsOptionToReadableString(Import::AudioFilenameExistsOption::Replace) == "Replace");
		REQUIRE(ImportHelper::audioFilenameExistsOptionToReadableString(Import::AudioFilenameExistsOption::UseExisting) == "Use Existing");
	}

	TEST_CASE("applyTemplateOptionToReadableString")
	{
		REQUIRE(ImportHelper::applyTemplateOptionToReadableString(Import::ApplyTemplateOption::Always) == "Always");
		REQUIRE(ImportHelper::applyTemplateOptionToReadableString(Import::ApplyTemplateOption::NewObjectCreationOnly) == "New Object Creation Only");
	}

	TEST_CASE("objectStatusToReadableString")
	{
		REQUIRE(ImportHelper::objectStatusToReadableString(Import::ObjectStatus::New) == "New");
		REQUIRE(ImportHelper::objectStatusToReadableString(Import::ObjectStatus::Replaced) == "Replaced");
		REQUIRE(ImportHelper::objectStatusToReadableString(Import::ObjectStatus::NewRenamed) == "New (Renamed)");
		REQUIRE(ImportHelper::objectStatusToReadableString(Import::ObjectStatus::NoChange) == "No Change");
	}

	TEST_CASE("wavStatusToReadableString")
	{
		REQUIRE(ImportHelper::wavStatusToReadableString(Import::WavStatus::New) == "New");
		REQUIRE(ImportHelper::wavStatusToReadableString(Import::WavStatus::Replaced) == "Replaced");
		REQUIRE(ImportHelper::wavStatusToReadableString(Import::WavStatus::Unknown) == "");
	}

	TEST_CASE("importPreviewItemsToHash")
	{
		SECTION("Equality Check")
		{
			auto itemCount = GENERATE(0, 1, 3, 1000);
			auto testItems = std::vector<Import::PreviewItem>();
			for(int index = 0; index < itemCount; index++)
			{
				auto testValue = TestImportPreviewItemValues(index);
				auto testItem = testValue.generateImportPreviewItem();

				testItems.emplace_back(testItem);
			}

			REQUIRE(ImportHelper::importPreviewItemsToHash(testItems) == ImportHelper::importPreviewItemsToHash(testItems));
		}
		SECTION("Difference Check")
		{
			auto evenIndex = GENERATE(2, 3, 2000, 2001);
			auto testItems1 = std::vector<Import::PreviewItem>();
			auto testItems2 = std::vector<Import::PreviewItem>();

			for(int index = 0; index < evenIndex; index++)
			{
				auto testValue = TestImportPreviewItemValues(index);
				auto testItem = testValue.generateImportPreviewItem();

				if(index % 2 == 0)
				{
					testItems1.emplace_back(testItem);
				}
				else
				{
					testItems2.emplace_back(testItem);
				}
			}

			REQUIRE(ImportHelper::importPreviewItemsToHash(testItems1) != ImportHelper::importPreviewItemsToHash(testItems2));
		}
	}
} // namespace AK::WwiseTransfer::Test
