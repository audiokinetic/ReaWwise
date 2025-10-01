/*----------------------------------------------------------------------------------------

Copyright (c) 2026 AUDIOKINETIC Inc.

This file is licensed to use under the license available at:
https://github.com/audiokinetic/ReaWwise/blob/main/License.txt (the "License").
You may not use this file except in compliance with the License.

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.  See the License for the
specific language governing permissions and limitations under the License.

----------------------------------------------------------------------------------------*/

#include "Helpers/StringHelper.h"

#include <catch2/catch_test_macros.hpp>

namespace AK::WwiseTransfer::Test
{
	TEST_CASE("truncateUtf8")
	{
		SECTION("Text within the limit is returned unchanged")
		{
			REQUIRE(StringHelper::truncateUtf8("abc", 3) == "abc");
			REQUIRE(StringHelper::truncateUtf8("", 0).empty());
		}

		SECTION("Text over the limit is cut and marked with its original size")
		{
			auto result = StringHelper::truncateUtf8(std::string(100, 'a'), 10);

			REQUIRE(result.rfind(std::string(10, 'a'), 0) == 0);
			REQUIRE(result.find("100 bytes total") != std::string::npos);
			REQUIRE(result.size() < 100);
		}

		SECTION("Multi-byte characters are not split")
		{
			// "aéé" is 5 bytes: 'a', then two 2-byte characters (0xC3 0xA9).
			const std::string text = "a\xC3\xA9\xC3\xA9";
			const std::string marker = "... [truncated, 5 bytes total]";

			REQUIRE(StringHelper::truncateUtf8(text, 1) == "a" + marker);
			REQUIRE(StringHelper::truncateUtf8(text, 2) == "a" + marker); // Would cut the first 'é' in half.
			REQUIRE(StringHelper::truncateUtf8(text, 3) == "a\xC3\xA9" + marker); // Falls between the two 'é'.
			REQUIRE(StringHelper::truncateUtf8(text, 4) == "a\xC3\xA9" + marker); // Would cut the second 'é' in half.
		}
	}
} // namespace AK::WwiseTransfer::Test
