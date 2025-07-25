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

#include "Helpers/PersistanceHelper.h"
#include "Model/Import.h"
#include "Model/IDs.h"
#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <juce_core/juce_core.h>
using namespace juce;

namespace AK::WwiseTransfer::Test
{
	struct TestHierarchyMappingNodeValues
	{
		juce::String objectName;
		bool objectNameValid = true;
		juce::String objectNameErrorMsg;
		Wwise::ObjectType objectType;
		juce::String propertyTemplatePath;
		bool propertyTemplatePathEnabled;
		juce::String language;
		juce::Identifier identifier;

		TestHierarchyMappingNodeValues(int index = 0)
		{
			juce::String strIndex(index);

			objectName = "TestName_" + strIndex;
			objectNameValid = true;
			objectNameErrorMsg = "Test Name Error Message " + strIndex;
			objectType = Wwise::ObjectType::RandomContainer;
			propertyTemplatePath = "test\\template\\path";
			propertyTemplatePathEnabled = false;
			language = "English";
			identifier = IDs::hierarchyMappingNode;
		}

		inline juce::ValueTree generateValueTree() const
		{
			juce::ValueTree valueTree(identifier);
			valueTree.setProperty(IDs::objectName, objectName, nullptr);
			valueTree.setProperty(IDs::objectNameValid, objectNameValid, nullptr);
			valueTree.setProperty(IDs::objectNameErrorMessage, objectNameErrorMsg, nullptr);
			valueTree.setProperty(IDs::objectType, juce::VariantConverter<Wwise::ObjectType>::toVar(objectType), nullptr);
			valueTree.setProperty(IDs::propertyTemplatePath, propertyTemplatePath, nullptr);
			valueTree.setProperty(IDs::propertyTemplatePathEnabled, propertyTemplatePathEnabled, nullptr);
			valueTree.setProperty(IDs::objectLanguage, language, nullptr);

			return valueTree;
		}

		inline juce::XmlElement generateXMLElement() const
		{
			auto xmlElement = juce::XmlElement(identifier);
			xmlElement.setAttribute(IDs::objectName, objectName);
			xmlElement.setAttribute(IDs::objectNameValid, objectNameValid);
			xmlElement.setAttribute(IDs::objectNameErrorMessage, objectNameErrorMsg);
			xmlElement.setAttribute(IDs::objectType, WwiseHelper::objectTypeToReadableString(objectType));
			xmlElement.setAttribute(IDs::propertyTemplatePath, propertyTemplatePath);
			xmlElement.setAttribute(IDs::propertyTemplatePathEnabled, propertyTemplatePathEnabled);
			xmlElement.setAttribute(IDs::objectLanguage, language);

			return xmlElement;
		}
	};

	inline void addPropertiesToRemove(juce::ValueTree valueTree)
	{
		valueTree.setProperty(IDs::objectTypeValid, true, nullptr);
		valueTree.setProperty(IDs::objectTypeErrorMessage, "Type Error Message", nullptr);
		valueTree.setProperty(IDs::propertyTemplatePathValid, true, nullptr);
		valueTree.setProperty(IDs::propertyTemplatePathErrorMessage, "Property Template Path Error Message", nullptr);
	}

	inline void testHierarchyMappingPresetDataEquality(const juce::XmlElement& presetData, const TestHierarchyMappingNodeValues& values)
	{
		REQUIRE(presetData.getTagName() == values.identifier.toString());
		REQUIRE(presetData.getStringAttribute(IDs::objectName.toString()) == values.objectName);
		REQUIRE(presetData.getStringAttribute(IDs::objectType.toString()) == juce::String(static_cast<int>(values.objectType)));
		REQUIRE(presetData.getStringAttribute(IDs::propertyTemplatePath.toString()) == values.propertyTemplatePath);
		REQUIRE(presetData.getStringAttribute(IDs::propertyTemplatePathEnabled.toString()) == juce::String(static_cast<int>(values.propertyTemplatePathEnabled)));
		REQUIRE(presetData.getStringAttribute(IDs::objectLanguage.toString()) == values.language);
	}

	inline void testHierarchyMappingPresetRemovedProperties(const juce::XmlElement& presetData)
	{
		REQUIRE(presetData.getStringAttribute(IDs::objectNameValid.toString()).isEmpty());
		REQUIRE(presetData.getStringAttribute(IDs::objectNameErrorMessage.toString()).isEmpty());
		REQUIRE(presetData.getStringAttribute(IDs::objectTypeValid.toString()).isEmpty());
		REQUIRE(presetData.getStringAttribute(IDs::objectTypeErrorMessage.toString()).isEmpty());
		REQUIRE(presetData.getStringAttribute(IDs::propertyTemplatePathValid.toString()).isEmpty());
		REQUIRE(presetData.getStringAttribute(IDs::propertyTemplatePathErrorMessage.toString()).isEmpty());
	}

	inline void testHierarchyMappingValueTreeEquality(juce::ValueTree valueTree, const TestHierarchyMappingNodeValues& values)
	{
		// PersistanceHelper::presetDataToHierarchyMapping sets hardcoded data to newly created valueTree
		REQUIRE(valueTree.getProperty(IDs::objectName) == values.objectName);
		REQUIRE(valueTree.getProperty(IDs::objectNameValid));
		REQUIRE(juce::VariantConverter<juce::String>::fromVar(valueTree.getProperty(IDs::objectNameErrorMessage)).isEmpty());
		REQUIRE(juce::VariantConverter<Wwise::ObjectType>::fromVar(valueTree.getProperty(IDs::objectType)) == values.objectType);
		REQUIRE(valueTree.getProperty(IDs::objectTypeValid));
		REQUIRE(juce::VariantConverter<juce::String>::fromVar(valueTree.getProperty(IDs::objectTypeErrorMessage)).isEmpty());
		REQUIRE(valueTree.getProperty(IDs::propertyTemplatePath) == values.propertyTemplatePath);
		REQUIRE(valueTree.getProperty(IDs::propertyTemplatePathEnabled).equals(values.propertyTemplatePathEnabled));
		REQUIRE(valueTree.getProperty(IDs::propertyTemplatePathValid));
		REQUIRE(juce::VariantConverter<juce::String>::fromVar(valueTree.getProperty(IDs::propertyTemplatePathErrorMessage)).isEmpty());
		REQUIRE(valueTree.getProperty(IDs::objectLanguage) == values.language);
	}
} // namespace AK::WwiseTransfer::Test