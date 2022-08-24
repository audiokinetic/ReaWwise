#include "Helpers/ImportHelper.h"
#include "Model/Import.h"
#include "Model/IDs.h"

#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <juce_core/juce_core.h>

namespace AK::WwiseTransfer::Test
{
	struct TestTreeHierarchyValues
	{
		juce::String objectName;
		bool objectNameValid = true;
		juce::String objectNameErrorMsg;
		Wwise::ObjectType objectType;
		bool typeValid;
		juce::String typeErrorMsg;
		juce::String propertyTemplatePath;
		bool propertyTemplatePathEnabled;
		bool propertyTemplatePathValid;
		juce::String propertyTemplatePathErrorMsg;
		juce::String language;

		TestTreeHierarchyValues(int index = 0)
		{
			juce::String strIndex(index);

			objectName = "TestName_" + strIndex;
			objectNameValid = true;
			objectNameErrorMsg = "Test Name Error Message " + strIndex;
			objectType = Wwise::ObjectType::RandomContainer;
			typeValid = true;
			typeErrorMsg = "Test Type Error Message " + strIndex;
			propertyTemplatePath = "";
			propertyTemplatePathEnabled = false;
			propertyTemplatePathValid = true;
			propertyTemplatePathErrorMsg = "Test Property Template Path Error Message " + strIndex;
			language = "English";
		}

		inline Import::HierarchyMappingNode generateHierarchyMappingNode() const
		{
			auto hierarchyMappingNode = Import::HierarchyMappingNode(
				objectName,
				objectNameValid,
				objectNameErrorMsg,
				objectType,
				typeValid,
				typeErrorMsg,
				propertyTemplatePath,
				propertyTemplatePathEnabled,
				propertyTemplatePathValid,
				propertyTemplatePathErrorMsg,
				language
			);

			return hierarchyMappingNode;
		}

		inline juce::ValueTree generateValueTree() const
		{
			juce::ValueTree valueTree(IDs::hierarchyMappingNode);
			valueTree.setProperty(IDs::objectName, objectName, nullptr);
			valueTree.setProperty(IDs::objectNameValid, objectNameValid, nullptr);
			valueTree.setProperty(IDs::objectNameErrorMessage, objectNameErrorMsg, nullptr);
			valueTree.setProperty(IDs::objectType, juce::VariantConverter<Wwise::ObjectType>::toVar(objectType), nullptr);
			valueTree.setProperty(IDs::objectTypeValid, typeValid, nullptr);
			valueTree.setProperty(IDs::objectTypeErrorMessage, typeErrorMsg, nullptr);
			valueTree.setProperty(IDs::propertyTemplatePath, propertyTemplatePath, nullptr);
			valueTree.setProperty(IDs::propertyTemplatePathEnabled, propertyTemplatePathEnabled, nullptr);
			valueTree.setProperty(IDs::propertyTemplatePathValid, propertyTemplatePathValid, nullptr);
			valueTree.setProperty(IDs::propertyTemplatePathErrorMessage, propertyTemplatePathErrorMsg, nullptr);
			valueTree.setProperty(IDs::objectLanguage, language, nullptr);

			return valueTree;
		}
	};

	struct TestTreePreviewItemValues
	{
		juce::String objectName;
		Wwise::ObjectType objectType;
		juce::String audioFilePath;
		Import::ObjectStatus objectStatus;
		Import::WavStatus wavStatus;

		TestTreePreviewItemValues(int index = 0)
		{
			juce::String strIndex(index);
			objectName = "TestName_" + strIndex;
			objectType = Wwise::ObjectType::RandomContainer;
			audioFilePath = "\\test\\audio\\file\\path";
			objectStatus = Import::ObjectStatus::New;
			wavStatus = Import::WavStatus::New;
		}

		inline juce::ValueTree generateValueTree() const
		{
			juce::ValueTree valueTree(IDs::hierarchyMappingNode);

			valueTree.setProperty(IDs::objectName, objectName, nullptr);
			valueTree.setProperty(IDs::audioFilePath, audioFilePath, nullptr);
			valueTree.setProperty(IDs::objectType, juce::VariantConverter<Wwise::ObjectType>::toVar(objectType), nullptr);
			valueTree.setProperty(IDs::wavStatus, juce::VariantConverter<Import::WavStatus>::toVar(wavStatus), nullptr);
			valueTree.setProperty(IDs::objectStatus, juce::VariantConverter<Import::ObjectStatus>::toVar(objectStatus), nullptr);

			return valueTree;
		}

		inline Import::PreviewItem generatePreviewItem() const
		{
			return {
				objectName,
				objectType,
				objectStatus,
				audioFilePath,
				wavStatus
			};
		}
	};

	struct TestImportItemValues
	{
		juce::String name;
		Wwise::ObjectType type;
		juce::String path;
		juce::String originalsSubFolder;
		juce::String audioFilePath;
		juce::String renderFilePath;

		TestImportItemValues(int index = 0)
		{
			juce::String strIndex(index);
			name = "TestName_" + strIndex;
			type = Wwise::ObjectType::PhysicalFolder;
			path = "\\test\\item\\path_" + strIndex;
			originalsSubFolder = "testFolder";
			audioFilePath = "\\test\\item\\audio\\path_" + strIndex;
			renderFilePath = "\\test\\item\\render\\path_" + strIndex;
		}

		inline Import::Item generateImportItem() const
		{
			return
			{
				name,
				type,
				path,
				originalsSubFolder,
				audioFilePath,
				renderFilePath
			};
		}
	};

	inline void testValueTreeEquality(const juce::ValueTree& valueTree, const TestTreeHierarchyValues& values)
	{
		REQUIRE(valueTree.getProperty(IDs::objectName) == values.objectName);
		REQUIRE(valueTree.getProperty(IDs::objectNameValid).equals(values.objectNameValid));
		REQUIRE(valueTree.getProperty(IDs::objectNameErrorMessage) == values.objectNameErrorMsg);
		REQUIRE(juce::VariantConverter<Wwise::ObjectType>::fromVar(valueTree.getProperty(IDs::objectType)) == values.objectType);
		REQUIRE(valueTree.getProperty(IDs::objectTypeValid).equals(values.typeValid));
		REQUIRE(valueTree.getProperty(IDs::objectTypeErrorMessage) == values.typeErrorMsg);
		REQUIRE(valueTree.getProperty(IDs::propertyTemplatePath) == values.propertyTemplatePath);
		REQUIRE(valueTree.getProperty(IDs::propertyTemplatePathEnabled).equals(values.propertyTemplatePathEnabled));
		REQUIRE(valueTree.getProperty(IDs::propertyTemplatePathValid).equals(values.propertyTemplatePathValid));
		REQUIRE(valueTree.getProperty(IDs::propertyTemplatePathErrorMessage) == values.propertyTemplatePathErrorMsg);
		REQUIRE(valueTree.getProperty(IDs::objectLanguage) == values.language);
	}

	inline void testHierarchyMappingEquality(const Import::HierarchyMappingNode& node, const TestTreeHierarchyValues& values)
	{
		REQUIRE(node.name == values.objectName);
		REQUIRE(node.nameValid == values.objectNameValid);
		REQUIRE(node.nameErrorMessage == values.objectNameErrorMsg);
		REQUIRE(node.type == values.objectType);
		REQUIRE(node.typeValid == values.typeValid);
		REQUIRE(node.typeErrorMessage == values.typeErrorMsg);
		REQUIRE(node.propertyTemplatePath == values.propertyTemplatePath);
		REQUIRE(node.propertyTemplatePathEnabled == values.propertyTemplatePathEnabled);
		REQUIRE(node.propertyTemplatePathValid == values.propertyTemplatePathValid);
		REQUIRE(node.propertyTemplatePathErrorMessage == values.propertyTemplatePathErrorMsg);
		REQUIRE(node.language == values.language);
	}
} // namespace AK::WwiseTransfer::Test
