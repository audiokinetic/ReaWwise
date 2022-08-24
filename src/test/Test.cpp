#include "Helpers/WaapiHelper.h"
#include "Helpers/WwiseHelper.h"

#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <juce_core/juce_core.h>
#include <memory>

namespace AK::WwiseTransfer
{
	class WaapiHelperTests
	{
	};

	class WwiseHelperTests
	{
	};

	class WwiseModelTests
	{
	};

#pragma region WaapiHelperTests
	TEST_CASE_METHOD(WaapiHelperTests, "WaapiHelper::executeWithRetry: retries for max attempts when function returns false")
	{
		int counter = 0;
		auto alwaysFails = [&counter]
		{
			counter++;
			return false;
		};

		WaapiHelper::executeWithRetry(alwaysFails, 250, 5);

		REQUIRE(counter == 5);
	}

	TEST_CASE_METHOD(WaapiHelperTests, "WaapiHelper::executeWithRetry: does not retry when function returns true")
	{
		int counter = 0;
		auto alwaysSucceeds = [&counter]
		{
			counter++;
			return true;
		};

		WaapiHelper::executeWithRetry(alwaysSucceeds, 250, 5);

		REQUIRE(counter == 1);
	}

	TEST_CASE_METHOD(WaapiHelperTests, "WaapiHelper::executeWithRetry: stops retrying as soon as function returns true, on second try")
	{
		int counter = 0;
		bool returnValue = true;
		auto succeedsOnSecondTry = [&counter, &returnValue]
		{
			counter++;
			returnValue = !returnValue;
			return returnValue;
		};

		WaapiHelper::executeWithRetry(succeedsOnSecondTry, 250, 5);

		REQUIRE(counter == 2);
	}
#pragma endregion WaapiHelperTests

#pragma region WwiseHelperTests
	TEST_CASE_METHOD(WwiseHelperTests, "WwiseHelper::pathToPathParts")
	{
		std::vector<juce::String> paths{
			"\\Actor-Mixer Hierarchy\\Default Work Unit\\object",
			"Actor-Mixer Hierarchy\\Default Work Unit\\object\\"};

		for(auto& path : paths)
		{
			auto parts = WwiseHelper::pathToPathParts(path);

			REQUIRE(parts.size() == (size_t)3);
			REQUIRE(parts[0] == juce::String("Actor-Mixer Hierarchy"));
			REQUIRE(parts[1] == juce::String("Default Work Unit"));
			REQUIRE(parts[2] == juce::String("object"));
		}
	}

	TEST_CASE_METHOD(WwiseHelperTests, "WwiseHelper::pathToObjectName")
	{
		std::vector<std::pair<juce::String, juce::String>> getObjectNameTestCases{
			{"\\Actor-Mixer Hierarchy\\Default Work Unit\\object", "object"},
			{"\\Actor-Mixer Hierarchy\\Default Work Unit\\<SoundSFX>object", "object"},
		};

		for(auto& testCase : getObjectNameTestCases)
		{
			REQUIRE(WwiseHelper::pathToObjectName(testCase.first) == testCase.second);
		}
	}

	TEST_CASE_METHOD(WwiseHelperTests, "WwiseHelper::pathToAncestorPaths")
	{
		juce::String path = "\\Actor-Mixer Hierarchy\\Default Work Unit\\object";

		auto ancestors = WwiseHelper::pathToAncestorPaths(path);

		REQUIRE(ancestors.size() == (size_t)2);
		REQUIRE(ancestors[0] == juce::String("\\Actor-Mixer Hierarchy"));
		REQUIRE(ancestors[1] == juce::String("\\Actor-Mixer Hierarchy\\Default Work Unit"));
	}

	TEST_CASE_METHOD(WwiseHelperTests, "WwiseHelper::getCommonAncestor")
	{
		juce::String path1 = "\\Actor-Mixer Hierarchy\\Default Work Unit\\Container";
		juce::String path2 = "\\Actor-Mixer Hierarchy\\Default Work Unit\\Container\\Object";
		juce::String path3 = "\\Actor-Mixer Hierarchy\\Default Work Unit\\Container2\\Object2";
		juce::String path4 = "\\Default Work Unit";

		REQUIRE(WwiseHelper::getCommonAncestor(path1, path2) == juce::String("\\Actor-Mixer Hierarchy\\Default Work Unit\\Container"));
		REQUIRE(WwiseHelper::getCommonAncestor(path2, path3) == juce::String("\\Actor-Mixer Hierarchy\\Default Work Unit"));
		REQUIRE(WwiseHelper::getCommonAncestor(path3, path4) == juce::String(""));
	}
#pragma endregion WwiseHelperTests

#pragma region WwiseModelTests
	TEST_CASE_METHOD(WwiseModelTests, "Wwise::Version")
	{
		Wwise::Version v2022_1_0_0{2022, 1, 0, 0};
		Wwise::Version v2021_2_1_0{2021, 2, 1, 0};
		Wwise::Version v2021_3_1_0{2021, 3, 1, 0};
		Wwise::Version v2021_2_2_0{2021, 2, 2, 0};
		Wwise::Version v2021_2_1_1{2021, 2, 1, 1};

		REQUIRE(v2022_1_0_0 >= v2022_1_0_0);
		REQUIRE(v2022_1_0_0 <= v2022_1_0_0);
		REQUIRE(v2022_1_0_0 == v2022_1_0_0);

		REQUIRE(v2022_1_0_0 > v2021_2_1_0);
		REQUIRE(v2022_1_0_0 >= v2021_2_1_0);
		REQUIRE(v2022_1_0_0 != v2021_2_1_0);

		REQUIRE(v2021_2_1_0 < v2021_3_1_0);
		REQUIRE(v2021_2_1_0 <= v2021_3_1_0);
		REQUIRE(v2021_2_1_0 != v2021_3_1_0);

		REQUIRE(v2021_2_1_0 < v2021_2_2_0);
		REQUIRE(v2021_2_1_0 != v2021_2_2_0);

		REQUIRE(v2021_2_1_0 < v2021_2_1_1);
		REQUIRE(v2021_2_1_0 <= v2021_2_1_1);
		REQUIRE(v2021_2_1_0 != v2021_2_1_1);
	}
#pragma endregion WwiseModelTests
} // namespace AK::WwiseTransfer
