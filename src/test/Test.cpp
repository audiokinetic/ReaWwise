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
