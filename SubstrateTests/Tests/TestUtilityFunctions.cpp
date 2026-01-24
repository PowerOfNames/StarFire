#include <catch2/catch_test_macros.hpp>

#define SUBSTRATE_ENABLE_DETAILS
#include "Substrate/UtilityFunctions.h"


TEST_CASE("UtilityFunctions AlignUp1_MaxBytes", "[Utility][AlignUp1_MaxBytes]")
{
	INFO("Max alignnment is " <<alignof(std::max_align_t));
	SECTION("AlignUp uint8_t")
	{
		REQUIRE(Substrate::AlignUp1_MaxBytes(0) == 0);
		REQUIRE(Substrate::AlignUp1_MaxBytes(1) == 1);
		REQUIRE(Substrate::AlignUp1_MaxBytes(2) == 2);
		REQUIRE(Substrate::AlignUp1_MaxBytes(3) == 4);
		REQUIRE(Substrate::AlignUp1_MaxBytes(5) == 8);
		REQUIRE(Substrate::AlignUp1_MaxBytes(8) == 8);
	}
}

TEST_CASE("UtilityFunctions IsPowerOfTwo", "[Utility][IsPowerOfTwo]")
{
	REQUIRE(Substrate::IsPowerOfTwo(0) == false);
	REQUIRE(Substrate::IsPowerOfTwo(1) == true);
	REQUIRE(Substrate::IsPowerOfTwo(2) == true);
	REQUIRE(Substrate::IsPowerOfTwo(32) == true);
	REQUIRE(Substrate::IsPowerOfTwo(128) == true);
	REQUIRE(Substrate::IsPowerOfTwo(178) == false);
	REQUIRE(Substrate::IsPowerOfTwo(UINT64_MAX) == false);
}

TEST_CASE("UtilityFunctions AlignUpToMultipleOfMinAlignment", "[Utility][AlignUpToMultipleOfMinAlignment]")
{
	SECTION("MinAlignment validity (must be power of two)")
	{
		REQUIRE_THROWS_AS(Substrate::AlignUpToMultipleOfMinAlignment(1, 3), Substrate::BadAlignmentException);
		REQUIRE_NOTHROW(Substrate::AlignUpToMultipleOfMinAlignment(1, 4));
		REQUIRE_NOTHROW(Substrate::AlignUpToMultipleOfMinAlignment(1, 8));
	}

	SECTION("Alignment check")
	{
		REQUIRE(Substrate::AlignUpToMultipleOfMinAlignment(1, 1) == 1);
		REQUIRE(Substrate::AlignUpToMultipleOfMinAlignment(1, 2) == 2);
		REQUIRE(Substrate::AlignUpToMultipleOfMinAlignment(2, 1) == 2);
		REQUIRE(Substrate::AlignUpToMultipleOfMinAlignment(30, 8) == 32);
	}
}