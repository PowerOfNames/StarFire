#include <catch2/catch_test_macros.hpp>

#define SUBSTRATE_ENABLE_DETAILS
#include "Substrate/PoolAllocator.h"
#include "Substrate/UtilityFunctions.h"


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

using Allocator = Substrate::PoolAllocator<TestStruct, uint32_t, AllocatorSize>;

TEST_CASE("Pool Allocator", "[Allocator][Pool]")
{
	SECTION("Creation")
	{
		Allocator allocator;
		REQUIRE(allocator.GetTotalMemory() == AllocatorSize);
		REQUIRE(allocator.GetUsedMemory() == 0);
		REQUIRE(allocator.GetMaxAllocationCount() == MaxAllocations);
		REQUIRE(allocator.GetCurrentAllocationCount() == 0);
		REQUIRE(allocator.GetTotalAllocationCount() == 0);
		REQUIRE(allocator.GetResetCount() == 0);
		REQUIRE(allocator.GetFreeHandleIndices().size() == MaxAllocations);
		

		REQUIRE(Allocator::MAX_BLOCK_COUNT == MaxAllocations);
		REQUIRE(Allocator::INDEX_BIT_COUNT == Substrate::Utility::Log2Up(85)); //should be 7
		REQUIRE(Allocator::GENERATION_BIT_COUNT == 32 - Substrate::Utility::Log2Up(85)); // should be 32 - 7 == 25
	}

	SECTION("Allocation")
	{
		Allocator allocator;
		uint32_t handle = allocator.Allocate();
		REQUIRE(handle == 0);
		REQUIRE(allocator.GetUsedMemory() == TestStructSize);
		REQUIRE(allocator.GetCurrentAllocationCount() == 1);
		REQUIRE(allocator.GetTotalAllocationCount() == 1);
	}

	SECTION("GetPointerFromHandle")
	{
		Allocator allocator;
		uint32_t handle = allocator.Allocate();
		TestStruct* ptr = allocator.GetPointerFromHandle(handle);
		REQUIRE(ptr != nullptr);
	}

	SECTION("Write to allocated block")
	{
		Allocator allocator;
		uint32_t handle = allocator.Allocate();
		TestStruct* ptr = allocator.GetPointerFromHandle(handle);
		ptr->a = 42;
		ptr->b = 3.14f;
		ptr->c = 'x';
		REQUIRE(ptr->a == 42);
		REQUIRE(ptr->b == 3.14f);
		REQUIRE(ptr->c == 'x');
	}

	SECTION("Free")
	{
		Allocator allocator;
		uint32_t handle = allocator.Allocate();
		TestStruct* ptr = allocator.GetPointerFromHandle(handle);
		allocator.Free(handle);

		REQUIRE(allocator.GetUsedMemory() == 0);
		REQUIRE(allocator.GetCurrentAllocationCount() == 0);
		REQUIRE(allocator.GetTotalAllocationCount() == 1);
	}

	SECTION("Free and reallocate")
	{
		Allocator allocator;
		uint32_t handle1 = allocator.Allocate();
		allocator.Free(handle1);
		uint32_t handle2 = allocator.Allocate();
		
		REQUIRE(handle1 != handle2); // Handle should have a different generation now
		REQUIRE(handle2 == 128);	// Handle index should be 0 again, but the first generation bit should be incremented by 1 -> 1 << 7 == 128

		REQUIRE(allocator.GetUsedMemory() == TestStructSize);
		REQUIRE(allocator.GetCurrentAllocationCount() == 1);
		REQUIRE(allocator.GetTotalAllocationCount() == 2);
	}

	SECTION("Allocate until full")
	{
		Allocator allocator;
		for (size_t i = 0; i < MaxAllocations; i++)
		{
			allocator.Allocate();
		}
		REQUIRE(allocator.GetUsedMemory() == MaxAllocations * TestStructSize);
		REQUIRE(allocator.GetCurrentAllocationCount() == MaxAllocations);
		REQUIRE(allocator.GetTotalAllocationCount() == MaxAllocations);
		REQUIRE_THROWS_AS(allocator.Allocate(), Substrate::AllocatorOutOfMemoryException);
	}

	SECTION("Allocate and reallocate until generations are exhausted")
	{
		Allocator allocator;
		constexpr size_t maxGenerations = (1 << Allocator::GENERATION_BIT_COUNT) - 1;
		uint32_t firstHandle;
		for (size_t gen = 0; gen < maxGenerations; gen++)
		{
			firstHandle = allocator.Allocate();
			allocator.Free(firstHandle);
		}
		REQUIRE(allocator.GetTotalAllocationCount() == maxGenerations);
		REQUIRE(allocator.GetCurrentAllocationCount() == 1);		// Last allocation is still active because it wont get freed again
		REQUIRE(allocator.GetUsedMemory() == TestStructSize);		// We dont free -> used memory should still be 1 block

		// Next allocation should use the next index, as all generations for index 0 are exhausted and it will not end up in the free list again
		uint32_t handle = allocator.Allocate();
		REQUIRE(handle == 1); // Next index
	}
}