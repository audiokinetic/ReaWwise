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

#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/trompeloeil.hpp>
#include "Core/WaapiClient.h"

namespace AK::WwiseTransfer::Test
{
	class MockWaapiClient : public AK::WwiseTransfer::WaapiClient
	{
	public:
		MAKE_MOCK4(connect, bool(char*, unsigned int, AK::WwiseAuthoringAPI::disconnectHandler_t, int));
	};

	TEST_CASE("connect example")
	{
		using trompeloeil::_;  // wild card for matching any value

		MockWaapiClient client;

		REQUIRE_CALL(client, connect(_, 8080, nullptr, -1))
			.RETURN(true);

		ALLOW_CALL(client, connect(_, 8000, nullptr, -1))
			.RETURN(false);

		REQUIRE(client.connect("testurl", 8080, nullptr, -1));
		REQUIRE_FALSE(client.connect("testurl", 8000, nullptr, -1));
	}
} // AK::WwiseTransfer::Test
