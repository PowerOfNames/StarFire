#include <catch2/catch_test_macros.hpp>

#define SUBSTRATE_ENABLE_DETAILS
#include "Substrate/LinearAllocator.h"
#include "Substrate/Exceptions.h"

#include <cstdint>

struct TestStruct	// 9 bytes -> aligned to 16 bytes
{
	uint32_t a;		// 4 bytes
	float b;		// 4 bytes
	char c;			// 1 byte
};

constexpr size_t TestStructSize = 12; // AlignUp1_MaxBytes(9, alignof(std::max_align_t)) == 16
constexpr size_t AllocatorSize = 1024;
constexpr size_t MaxAllocations = AllocatorSize / TestStructSize; // 1024 / 12 == 85 | 3.333


TEST_CASE("Linear Allocator", "[Allocator][Linear]")
{
	SECTION("Creation")
	{
		Substrate::LinearAllocator<TestStruct> allocator(AllocatorSize);
		REQUIRE(allocator.GetTotalMemory() == AllocatorSize);
		REQUIRE(allocator.GetUsedMemory() == 0);
		REQUIRE(allocator.GetAllocationCount() == 0);
	}

	SECTION("Allocation")
	{
		Substrate::LinearAllocator<TestStruct> allocator(1024);
		TestStruct* ptr = allocator.Allocate();
		REQUIRE(allocator.GetUsedMemory() == 12);
		REQUIRE(allocator.GetAllocationCount() == 1);
	}

	SECTION("Array allocation+access check")
	{
		Substrate::LinearAllocator<TestStruct> allocator(1024);
		auto structs = allocator.AllocateArray(2);
		REQUIRE(allocator.GetUsedMemory() == 2 * TestStructSize);
		REQUIRE(allocator.GetAllocationCount() == 2);
		REQUIRE_THROWS_AS(structs[3], Substrate::ArrayIndexOutOfBoundsException);
	}

	SECTION("Allocator full")
	{
		Substrate::LinearAllocator<TestStruct> allocator(16);
		TestStruct* ptr1 = allocator.Allocate();
		REQUIRE(allocator.Allocate() == nullptr);
	}

	SECTION("Array allocation too large")
	{
		Substrate::LinearAllocator<TestStruct> allocator(16);
		auto structs = allocator.AllocateArray(2);
		REQUIRE(structs.Data == nullptr);
		REQUIRE(structs.Count == 0);
	}

	SECTION("Allocator reset.")
	{
		Substrate::LinearAllocator<TestStruct> allocator(1024);
		TestStruct* ptr1 = allocator.Allocate();
		REQUIRE(allocator.GetUsedMemory() == TestStructSize);
		REQUIRE(allocator.GetAllocationCount() == 1);
		allocator.Reset();
		REQUIRE(allocator.GetUsedMemory() == 0);
		REQUIRE(allocator.GetAllocationCount() == 0);
	}
}