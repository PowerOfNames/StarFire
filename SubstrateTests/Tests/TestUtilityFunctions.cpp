#include <catch2/catch_test_macros.hpp>

#define SUBSTRATE_ENABLE_DETAILS
#include "Substrate/UtilityFunctions.h"


TEST_CASE("UtilityFunctions AlignUp1_MaxBytes", "[Utility][AlignUp1_MaxBytes]")
{
	INFO("Max alignnment is " <<alignof(std::max_align_t));
	SECTION("AlignUp uint8_t")
	{
		REQUIRE(Substrate::Utility::AlignUp1_MaxBytes(0) == 0);
		REQUIRE(Substrate::Utility::AlignUp1_MaxBytes(1) == 1);
		REQUIRE(Substrate::Utility::AlignUp1_MaxBytes(2) == 2);
		REQUIRE(Substrate::Utility::AlignUp1_MaxBytes(3) == 4);
		REQUIRE(Substrate::Utility::AlignUp1_MaxBytes(5) == 8);
		REQUIRE(Substrate::Utility::AlignUp1_MaxBytes(8) == 8);
	}
}

TEST_CASE("UtilityFunctions IsPowerOfTwo", "[Utility][IsPowerOfTwo]")
{
	REQUIRE(Substrate::Utility::IsPowerOfTwo(0) == false);
	REQUIRE(Substrate::Utility::IsPowerOfTwo(1) == true);
	REQUIRE(Substrate::Utility::IsPowerOfTwo(2) == true);
	REQUIRE(Substrate::Utility::IsPowerOfTwo(32) == true);
	REQUIRE(Substrate::Utility::IsPowerOfTwo(128) == true);
	REQUIRE(Substrate::Utility::IsPowerOfTwo(178) == false);
	REQUIRE(Substrate::Utility::IsPowerOfTwo(UINT64_MAX) == false);
}

TEST_CASE("UtilityFunctions AlignUpToMultipleOfMinAlignment", "[Utility][AlignUpToMultipleOfMinAlignment]")
{
	SECTION("MinAlignment validity (must be power of two)")
	{
		REQUIRE_THROWS_AS(Substrate::Utility::AlignUpToMultipleOfMinAlignment(1, 3), Substrate::BadAlignmentException);
		REQUIRE_NOTHROW(Substrate::Utility::AlignUpToMultipleOfMinAlignment(1, 4));
		REQUIRE_NOTHROW(Substrate::Utility::AlignUpToMultipleOfMinAlignment(1, 8));
	}

	SECTION("Alignment check")
	{
		REQUIRE(Substrate::Utility::AlignUpToMultipleOfMinAlignment(1, 1) == 1);
		REQUIRE(Substrate::Utility::AlignUpToMultipleOfMinAlignment(1, 2) == 2);
		REQUIRE(Substrate::Utility::AlignUpToMultipleOfMinAlignment(2, 1) == 2);
		REQUIRE(Substrate::Utility::AlignUpToMultipleOfMinAlignment(30, 8) == 32);
	}
}

TEST_CASE("UtilityFunctions Log2", "[Utility][Log2]")
{
	REQUIRE_THROWS_AS(Substrate::Utility::Log2(0), std::invalid_argument);

	REQUIRE(Substrate::Utility::Log2(1) == 0);
	REQUIRE(Substrate::Utility::Log2(2) == 1);
	REQUIRE(Substrate::Utility::Log2(3) == 1);
	REQUIRE(Substrate::Utility::Log2(128) == 7);
	REQUIRE(Substrate::Utility::Log2(512) == 9);
	REQUIRE(Substrate::Utility::Log2(511) == 8);
	REQUIRE(Substrate::Utility::Log2(1024) == 10);
	REQUIRE(Substrate::Utility::Log2(1023) == 9);
}

TEST_CASE("UtilityFunctions Log2Up", "[Utility][Log2Up]")
{
	REQUIRE_THROWS_AS(Substrate::Utility::Log2Up(0), std::invalid_argument);
	REQUIRE(Substrate::Utility::Log2Up(1) == 0);
	REQUIRE(Substrate::Utility::Log2Up(2) == 1);
	REQUIRE(Substrate::Utility::Log2Up(3) == 2);
	REQUIRE(Substrate::Utility::Log2Up(512) == 9);
	REQUIRE(Substrate::Utility::Log2Up(511) == 9);
	REQUIRE(Substrate::Utility::Log2Up(1024) == 10);
	REQUIRE(Substrate::Utility::Log2Up(1023) == 10);
}