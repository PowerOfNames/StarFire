#include <catch2/catch_test_macros.hpp>

#define SUBSTRATE_ENABLE_DETAILS
#include "Substrate/StackAllocator.h"


struct TestStruct	// 9 bytes -> aligned to 12 bytes
{
	uint32_t a;		// 4 bytes
	float b;		// 4 bytes
	char c;			// 1 byte
};

constexpr size_t TestStructSize = 12;
constexpr size_t AllocatorSize = 1024;
constexpr size_t MaxAllocations = AllocatorSize / TestStructSize; // 1024 / 12 == 85 | 3.333
constexpr size_t BlockSizeWithPadding = 12 + 4;

TEST_CASE("Stack Allocator", "[Allocator][Stack]")
{
	SECTION("Creation")
	{
		Substrate::StackAllocator allocator(AllocatorSize);
		REQUIRE(allocator.GetTotalMemory() == AllocatorSize);
		REQUIRE(allocator.GetUsedMemory() == 0);
		REQUIRE(allocator.GetAllocationCount() == 0);
	}

	SECTION("Allocation")
	{
		Substrate::StackAllocator allocator(1024);
		TestStruct* ptr = allocator.Allocate<TestStruct>();
		REQUIRE(allocator.GetUsedMemory() == BlockSizeWithPadding + sizeof(uint64_t)); // 3 bytes for the struct, 4 bytes padding to align to 8 bytes, 8 bytes for the header
		REQUIRE(allocator.GetAllocationCount() == 1);
	}

	SECTION("Allocator full")
	{
		Substrate::StackAllocator allocator(16);
		TestStruct* ptr1 = allocator.Allocate<TestStruct>();
		REQUIRE(allocator.Allocate<TestStruct>() == nullptr);
	}

	SECTION("Allocator pop")
	{
		Substrate::StackAllocator allocator(1024);
		TestStruct* ptr1 = allocator.Allocate<TestStruct>();
		REQUIRE(allocator.GetUsedMemory() == BlockSizeWithPadding + sizeof(uint64_t));
		REQUIRE(allocator.GetAllocationCount() == 1);
		TestStruct* ptr2 = allocator.Allocate<TestStruct>();
		REQUIRE(allocator.GetUsedMemory() == (BlockSizeWithPadding + sizeof(uint64_t)) * 2);
		REQUIRE(allocator.GetAllocationCount() == 2);
		allocator.Pop();
		REQUIRE(allocator.GetUsedMemory() == BlockSizeWithPadding + sizeof(uint64_t));
		REQUIRE(allocator.GetAllocationCount() == 1);
	}

	SECTION("Allocator reset.")
	{
		Substrate::StackAllocator allocator(1024);
		TestStruct* ptr1 = allocator.Allocate<TestStruct>();
		REQUIRE(allocator.GetUsedMemory() == BlockSizeWithPadding + sizeof(uint64_t));
		REQUIRE(allocator.GetAllocationCount() == 1);
		allocator.Reset();
		REQUIRE(allocator.GetUsedMemory() == 0);
		REQUIRE(allocator.GetAllocationCount() == 0);
	}
}