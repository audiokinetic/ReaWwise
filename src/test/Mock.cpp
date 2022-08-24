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
