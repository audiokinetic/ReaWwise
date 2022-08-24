#include "WwiseHelperTests.h"

namespace AK::WwiseTransfer::Test
{
	TEST_CASE("objectTypeToReadableString")
	{
		for (const auto& typeStringPair : objectTypeStringMap)
		{
			REQUIRE(WwiseHelper::objectTypeToReadableString(typeStringPair.first) == typeStringPair.second);
		}
	}

	TEST_CASE("stringToObjectType")
	{
		for (const auto& typeStringPair : objectTypeStringMap)
		{
			REQUIRE(WwiseHelper::stringToObjectType(typeStringPair.second) == typeStringPair.first);
		}
	}

	TEST_CASE("buildObjectPathNode")
	{
		const auto testName = "testObject";

		for (const auto& objectType : objectTypes)
		{
			auto expectedResult = "\\<" + WwiseHelper::objectTypeToReadableString(objectType) + ">" + testName;
			REQUIRE(WwiseHelper::buildObjectPathNode(objectType, testName) == expectedResult);
		}
	}

	TEST_CASE("pathToPathWithoutObjectTypes")
	{
		const auto testName = "testObject";
		const auto expectedResult = "\\testObject";

		for (const auto& objectType : objectTypes)
		{
			auto buildObjectPath = WwiseHelper::buildObjectPathNode(objectType, testName);
			REQUIRE(WwiseHelper::pathToPathWithoutObjectTypes(buildObjectPath) == expectedResult);
		}
	}

	TEST_CASE("pathToPathParts")
	{
		SECTION("Wwise Object Directory")
		{
			auto testPath = "\\test\\path\\directory\\multiple";

			REQUIRE(WwiseHelper::pathToPathParts(testPath) == std::vector<juce::String> { "test", "path", "directory", "multiple" });
		}
		SECTION("Wwise Object Type Directory")
		{
			auto testPath = "\\<testObject>testName\\path\\directory\\multiple";

			REQUIRE(WwiseHelper::pathToPathParts(testPath) == std::vector<juce::String> { "<testObject>testName", "path", "directory", "multiple" });
		}

	}

	TEST_CASE("pathToAncestorPaths")
	{
		SECTION("Wwise Object Directory")
		{
			auto testPath = "\\test\\path\\directory\\multiple";

			REQUIRE(WwiseHelper::pathToAncestorPaths(testPath) == std::vector<juce::String> { "\\test", "\\test\\path", "\\test\\path\\directory" });
		}

		SECTION("Wwise Object Type Directory")
		{
			auto testPath = "\\<testObject>testName\\path\\directory\\multiple";

			REQUIRE(WwiseHelper::pathToAncestorPaths(testPath) == std::vector<juce::String> { "\\<testObject>testName", "\\<testObject>testName\\path", "\\<testObject>testName\\path\\directory" });
		}
	}

	TEST_CASE("pathToObjectName")
	{
		SECTION("Wwise Object Directory")
		{
			auto testPath = "\\test\\path\\directory\\multiple";

			REQUIRE(WwiseHelper::pathToObjectName(testPath) == "multiple");
		}

		SECTION("Wwise Object Type Directory")
		{
			auto testPath = "\\<testObject>testName\\path\\directory\\multiple";

			REQUIRE(WwiseHelper::pathToObjectName(testPath) == "multiple");
		}
	}

	TEST_CASE("pathToObjectType")
	{
		auto testName = "testObjectName";

		SECTION("Wwise::ObjectType")
		{
			for (const auto& objectType : objectTypes)
			{
				auto buildObjectPath = WwiseHelper::buildObjectPathNode(objectType, testName);

				REQUIRE(WwiseHelper::pathToObjectType(buildObjectPath) == objectType);
			}
		}

		SECTION("Actor-Mixer Hierarchy")
		{
			auto testPath = "\\Actor-Mixer Hierarchy";

			REQUIRE(WwiseHelper::pathToObjectType(testPath) == Wwise::ObjectType::ActorMixer);
		}
	}

	TEST_CASE("versionToValueTree")
	{
		SECTION("Non-Null Values")
		{
			auto year = 2022;
			auto major = 1;
			auto minor = 10;
			auto build = 123456;

			auto version = Wwise::Version
			{
				year,
				major,
				minor,
				build
			};

			auto versionTree = WwiseHelper::versionToValueTree(version);

			REQUIRE(juce::VariantConverter<int>::fromVar(versionTree.getProperty(IDs::year)) == year);
			REQUIRE(juce::VariantConverter<int>::fromVar(versionTree.getProperty(IDs::major)) == major);
			REQUIRE(juce::VariantConverter<int>::fromVar(versionTree.getProperty(IDs::minor)) == minor);
			REQUIRE(juce::VariantConverter<int>::fromVar(versionTree.getProperty(IDs::build)) == build);
		}

		SECTION("Null Values")
		{
			auto version = Wwise::Version();

			auto versionTree = WwiseHelper::versionToValueTree(version);

			juce::var nullVar;

			REQUIRE(versionTree.getProperty(IDs::year) == nullVar);
			REQUIRE(versionTree.getProperty(IDs::major) == nullVar);
			REQUIRE(versionTree.getProperty(IDs::minor) == nullVar);
			REQUIRE(versionTree.getProperty(IDs::build) == nullVar);
		}
	}

	TEST_CASE("valueTreeToVersion")
	{
		SECTION("Non-Null Values")
		{
			auto year = 2022;
			auto major = 1;
			auto minor = 10;
			auto build = 123456;

			auto versionTree = juce::ValueTree(IDs::version);

			versionTree.setProperty(IDs::year, year, nullptr);
			versionTree.setProperty(IDs::major, major, nullptr);
			versionTree.setProperty(IDs::minor, minor, nullptr);
			versionTree.setProperty(IDs::build, build, nullptr);

			auto version = WwiseHelper::valueTreeToVersion(versionTree);

			REQUIRE(version.year == year);
			REQUIRE(version.major == major);
			REQUIRE(version.minor == minor);
			REQUIRE(version.build == build);
		}

		SECTION("Null Values")
		{
			auto versionTree = juce::ValueTree(IDs::version);

			auto version = WwiseHelper::valueTreeToVersion(versionTree);

			REQUIRE(version.year == 0);
			REQUIRE(version.major == 0);
			REQUIRE(version.minor == 0);
			REQUIRE(version.build == 0);
		}
	}

	TEST_CASE("languagesToValueTree")
	{
		auto languages = std::vector<juce::String>();
		SECTION("Empty")
		{
			auto languageTreeRoot = WwiseHelper::languagesToValueTree(languages);

			REQUIRE(languageTreeRoot.getNumChildren() == 0);
			REQUIRE(languageTreeRoot.getType() == IDs::languages);
		}

		SECTION("Non-Empty")
		{
			languages = { "English", "French", "Ukranian" };

			auto languageTreeRoot = WwiseHelper::languagesToValueTree(languages);

			REQUIRE(languageTreeRoot.getNumChildren() == languages.size());
			REQUIRE(languageTreeRoot.getType() == IDs::languages);

			for (int index = 0; index < languages.size(); index++)
			{
				REQUIRE(languageTreeRoot.getChild(index).getProperty(IDs::languageName) == languages[index]);
				REQUIRE(languageTreeRoot.getChild(index).getType() == IDs::language);
			}
		}
	}

	TEST_CASE("valueTreeToLanguages")
	{
		auto languagesValueTree = juce::ValueTree(IDs::languages);

		SECTION("Empty")
		{
			auto langauges = WwiseHelper::valueTreeToLanguages(languagesValueTree);

			REQUIRE_THAT(langauges, Catch::Matchers::IsEmpty());
		}

		SECTION("Non-Empty")
		{
			auto languages = std::vector<juce::String>{ "English", "French", "Ukranian" };

			for (const auto& language : languages)
			{
				auto childValueTree = juce::ValueTree(IDs::language);
				childValueTree.setProperty(IDs::languageName, language, nullptr);
				languagesValueTree.appendChild(childValueTree, nullptr);
			}

			REQUIRE_THAT(WwiseHelper::valueTreeToLanguages(languagesValueTree), Catch::Matchers::Equals(languages));
		}
	}

	TEST_CASE("getCommonAncestor")
	{
		SECTION("Folder Directory")
		{
			auto testPath1 = "\\test\\common\\ancestor\\folder1";
			auto testPath2 = "\\test\\common\\ancestor\\folder2";

			REQUIRE(WwiseHelper::getCommonAncestor(testPath1, testPath2) == "\\test\\common\\ancestor");
		}

		SECTION("Object Type Directory")
		{
			auto testPath1 = "\\test\\<object type>testName\\ancestor\\folder1";
			auto testPath2 = "\\test\\<object type>testName\\ancestor\\folder2";

			REQUIRE(WwiseHelper::getCommonAncestor(testPath1, testPath2) == "\\test\\<object type>testName\\ancestor");
		}

		SECTION("No Common Ancestors")
		{
			auto testPath1 = "\\no\\common\\ancestors";
			auto testPath2 = "\\without\\same\\subpath";

			REQUIRE(WwiseHelper::getCommonAncestor(testPath1, testPath2) == "");
		}
	}
} // namespace AK::WwiseTransfer:Test
