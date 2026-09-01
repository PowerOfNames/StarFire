#include <catch2/catch_test_macros.hpp>

#define SUBSTRATE_ENABLE_DETAILS
#include "Substrate/BaseHandle.h"

using TestHandle16_4_12 = Substrate::DefineHandle<4, 12, uint16_t>;

TEST_CASE("AllocationHandle Generation creation and validation", "[AllocationHandle][Generation]")
{
	SECTION("Create generation mask")
	{
		TestHandle16_4_12 handle(0);
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
		using TestHandle16_0_16 = Substrate::DefineHandle<0, 16, uint16_t>;
		TestHandle16_0_16 handle = TestHandle16_0_16(0);
		REQUIRE(handle.GetGenerationMask() == 0x0000);
		REQUIRE(handle.IsValid() == true);
	}

	// "Max value mask" (DefineHandle<16, 0, uint16_t>) was removed on 2026-09-01.
	// An all-ones generation mask leaves zero index bits, so the handle can address
	// nothing and every instance read as invalid. That is now a static_assert in
	// BaseHandle -- the instantiation itself is ill-formed, so there is no object
	// left to assert against from here. Do not re-add it; it will not compile.

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

TEST_CASE("AllocationHandle Index handling", "[AllocationHandle][Index]")
{
	SECTION("Index mask")
	{
		constexpr uint16_t indexMask = Substrate::HandleHelpers::GenerateIndexMask<uint16_t>(12);
		REQUIRE(indexMask == 0xFFF);
	}

	SECTION("Invalid handle index bits check.")
	{
		REQUIRE(TestHandle16_4_12::INVALID_HANDLE == 0xFFFF);
	}

	SECTION("Creation")
	{
		TestHandle16_4_12 handle = TestHandle16_4_12(5);
		REQUIRE(handle.Index() == 5);
	}

	SECTION("Generation increase")
	{
		TestHandle16_4_12 handle = TestHandle16_4_12(5);
		handle = handle.IncrementGeneration();
		REQUIRE(handle.Index() == 5);
	}

	SECTION("Max index")
	{
		TestHandle16_4_12 handle = TestHandle16_4_12(0);
		REQUIRE(handle.GetMaxIndexValue() == 0xFFF);
	}

	// "Zero index mask" removed on 2026-09-01 for the same reason as "Max value mask"
	// above: it instantiated DefineHandle<16, 0, uint16_t>, which the BaseHandle
	// static_assert now rejects at compile time.

	SECTION("Max value index mask")
	{
		using TestHandle16_0_16 = Substrate::DefineHandle<0, 16, uint16_t>;
		TestHandle16_0_16 handle = TestHandle16_0_16(0);
		REQUIRE(handle.GetIndexMask() == 0xFFFF);
		REQUIRE(handle.IsValid() == true);
	}

	SECTION("Overflow protection")
	{
		REQUIRE_THROWS_AS(TestHandle16_4_12(0xFFF+1), Substrate::HandleBitsOverflowException);
	}

	SECTION("Move construction and assignement")
	{
		TestHandle16_4_12 handle = TestHandle16_4_12(5);
		TestHandle16_4_12 otherHandle = std::move(handle);
		REQUIRE(otherHandle.Index() == 5);
		handle = std::move(otherHandle);
		REQUIRE(handle.Index() == 5);
	}
}