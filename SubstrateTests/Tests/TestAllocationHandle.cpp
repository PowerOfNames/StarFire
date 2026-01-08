#include <catch2/catch_test_macros.hpp>

#define SUBSTRATE_ENABLE_DETAILS
#include "Substrate/AllocationHandle.h"

using TestHandle16_4_12 = Substrate::GenerateHandle<4, 12, uint16_t>::Type;


TEST_CASE("AllocationHandle Generation creation and validation", "[AllocationHandle][Generation]")
{
	SECTION("Create generation mask")
	{
		TestHandle16_4_12 handle = TestHandle16_4_12(0);
		REQUIRE(handle.GetGenerationMask() == 0xF000);
	}

	SECTION("Get generation")
	{
		TestHandle16_4_12 handle = TestHandle16_4_12(0);
		REQUIRE(handle.Generation() == 0);
	}

	SECTION("Increase generation")
	{
		TestHandle16_4_12 handle = TestHandle16_4_12(0);
		TestHandle16_4_12 agedHandle = handle.IncrementGeneration();
		REQUIRE(agedHandle.EqualsRaw(0x1000)); // 0001 0000 0000 0000 in binary
		REQUIRE_FALSE(agedHandle.EqualsRaw(0x2000)); // 0001 0000 0000 0000 in binary
		REQUIRE(agedHandle.Generation() == 1);
		REQUIRE(handle.Generation() == 0);		//Should not be touched
	}

	SECTION("Increase generation stepsize always by on least significant bit")
	{
		TestHandle16_4_12 handle = TestHandle16_4_12(0);
		for (size_t i = 0; i < 5; i++)
		{
			handle = handle.IncrementGeneration();
		}
		REQUIRE(handle.EqualsRaw(0x5000)); // 0101 0000 0000 0000 in binary
		REQUIRE(handle.Generation() == 5);
	}

	SECTION("Generation equality check")
	{
		TestHandle16_4_12 handleA = TestHandle16_4_12(0);
		TestHandle16_4_12 handleB = TestHandle16_4_12(5);
		REQUIRE(handleA.EqualsGeneration(handleB) == true);
		TestHandle16_4_12 agedA = handleA.IncrementGeneration(); // 0x2005 in hex
		REQUIRE(handleA.EqualsGeneration(agedA) == false);
	}

	SECTION("Max generations")
	{
		TestHandle16_4_12 handle = TestHandle16_4_12(0);
		REQUIRE(handle.GetMaxGenerationValue() == 0xF); // 2^generationBitCount = 15
	}

	SECTION("Handle validity via generation")
	{
		TestHandle16_4_12 handle = TestHandle16_4_12(0);
		handle = handle.IncrementGeneration();
		REQUIRE(handle.IsValid() == true);
		for (size_t i = 0; i < 14; i++)
		{
			handle = handle.IncrementGeneration();
		}
		REQUIRE(handle.IsValid() == false);
	}

	SECTION("Zero mask")
	{
		using TestHandle16_0_16 = Substrate::GenerateHandle<0, 16, uint16_t>::Type;
		TestHandle16_0_16 handle = TestHandle16_0_16(0);
		REQUIRE(handle.IsValid() == false);
		REQUIRE(handle.GetGenerationMask() == 0x0000);
	}

	SECTION("Max value mask")
	{
		using TestHandle16_16_0 = Substrate::GenerateHandle<16, 0, uint16_t>::Type;
		TestHandle16_16_0 handle = TestHandle16_16_0(0);
		REQUIRE(handle.IsValid() == true);
		REQUIRE(handle.GetGenerationMask() == 0xFFFF);
	}

	SECTION("Overflow protection")
	{
		TestHandle16_4_12 handle = TestHandle16_4_12(0);
		handle = handle.IncrementGeneration();
		REQUIRE(handle.IsValid() == true);
		for (size_t i = 0; i < 14; i++)
		{
			handle = handle.IncrementGeneration();
		}
		handle = handle.IncrementGeneration();
		REQUIRE(handle.IsValid() == false);
	}
	
	SECTION("Move construction and assignement")
	{
		TestHandle16_4_12 handle = TestHandle16_4_12(0);
		handle = handle.IncrementGeneration();
		TestHandle16_4_12 otherHandle = std::move(handle);
		REQUIRE(otherHandle.Generation() == 1);
		handle = std::move(otherHandle);
		REQUIRE(handle.Generation() == 1);
	}
}