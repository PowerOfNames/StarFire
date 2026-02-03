#include <catch2/catch_test_macros.hpp>

#define SUBSTRATE_ENABLE_DETAILS
#include "Substrate/LinearAllocator.h"
#include "Substrate/Exceptions.h"

#include <cstdint>

struct TestStruct	// 9 bytes -> aligned to 12 bytes
{
	uint32_t a;		// 4 bytes
	float b;		// 4 bytes
	char c;			// 1 byte
};

constexpr size_t TestStructSize = 12;
constexpr size_t AllocatorSize = 1024;
constexpr size_t MaxAllocations = AllocatorSize / TestStructSize; // 1024 / 12 == 85 | 3.333

using Allocator = Substrate::LinearAllocator<TestStruct, AllocatorSize>;
using Allocator16 = Substrate::LinearAllocator<TestStruct, 16>;

TEST_CASE("Linear Allocator", "[Allocator][Linear]")
{
	SECTION("Creation")
	{
		Allocator allocator;
		REQUIRE(allocator.GetTotalMemory() == AllocatorSize);
		REQUIRE(allocator.GetUsedMemory() == 0);
		REQUIRE(allocator.GetMaxAllocationCount() == MaxAllocations);
		REQUIRE(allocator.GetTotalAllocationCount() == 0);
		REQUIRE(allocator.GetCurrentAllocationCount() == 0);
		REQUIRE(allocator.GetResetCount() == 0);
	}

	SECTION("Allocation")
	{
		Allocator allocator;
		TestStruct* ptr = allocator.Allocate();
		REQUIRE(allocator.GetUsedMemory() == 12);
		REQUIRE(allocator.GetCurrentAllocationCount() == 1);
		REQUIRE(allocator.GetTotalAllocationCount() == 1);
	}

	SECTION("Array allocation+access check")
	{
		Allocator allocator;
		auto structs = allocator.AllocateArray(2);
		REQUIRE(allocator.GetUsedMemory() == 2 * TestStructSize);
		REQUIRE(allocator.GetCurrentAllocationCount() == 2);
		REQUIRE(allocator.GetTotalAllocationCount() == 2);
		REQUIRE_THROWS_AS(structs[3], Substrate::ArrayIndexOutOfBoundsException);
	}

	SECTION("Allocator full")
	{
		Allocator16 allocator;
		TestStruct* ptr1 = allocator.Allocate();
		REQUIRE(allocator.Allocate() == nullptr);
	}

	SECTION("Array allocation too large")
	{
		Allocator16 allocator;
		auto structs = allocator.AllocateArray(2);
		REQUIRE(structs.Data == nullptr);
		REQUIRE(structs.Count == 0);
	}

	SECTION("Allocator reset.")
	{
		Allocator allocator;
		TestStruct* ptr1 = allocator.Allocate();
		REQUIRE(allocator.GetUsedMemory() == TestStructSize);
		REQUIRE(allocator.GetCurrentAllocationCount() == 1);
		REQUIRE(allocator.GetTotalAllocationCount() == 1);
		allocator.Reset();
		REQUIRE(allocator.GetUsedMemory() == 0);
		REQUIRE(allocator.GetCurrentAllocationCount() == 0);
		REQUIRE(allocator.GetTotalAllocationCount() == 1);
		REQUIRE(allocator.GetResetCount() == 1);
	}
}