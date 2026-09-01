#include <catch2/catch_test_macros.hpp>

#define SUBSTRATE_ENABLE_DETAILS
#include "Substrate/PoolAllocator.h"
#include "Substrate/UtilityFunctions.h"

#include <cstdint>
#include <vector>


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

// Raw value one generation step is worth: the generation sits above the index bits,
// so incrementing it adds (1 << INDEX_BIT_COUNT) to the raw handle.
constexpr uint32_t GenerationStep = 1u << Allocator::INDEX_BIT_COUNT; // 1 << 7 == 128

// A deliberately tiny pool for the generation-exhaustion cases. 36 / 12 == 3 blocks,
// so INDEX_BIT_COUNT == Log2Up(3) == 2 and GENERATION_BIT_COUNT == 16 - 2 == 14.
// That is 16383 generations per slot instead of the 33.5 million a uint32_t handle
// gives, which keeps these cases in the millisecond range.
constexpr size_t SmallAllocatorSize = 3 * TestStructSize;
using SmallAllocator = Substrate::PoolAllocator<TestStruct, uint16_t, SmallAllocatorSize>;

// 4 blocks == an exact power of two, so the highest index (3) equals the index mask
// (INDEX_BIT_COUNT == Log2Up(4) == 2). That slot used to read as invalid purely from
// its index bits, which made it allocatable but never freeable. Aurora's pools are
// this shape at 1024 blocks, so the case is worth pinning.
constexpr size_t PowerOfTwoAllocatorSize = 4 * TestStructSize;
using PowerOfTwoAllocator = Substrate::PoolAllocator<TestStruct, uint16_t, PowerOfTwoAllocatorSize>;


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
		REQUIRE(allocator.GetMaxedGenerationCount() == 0);
		REQUIRE(allocator.GetFreeHandleCount() == MaxAllocations);


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
		REQUIRE(allocator.GetFreeHandleCount() == MaxAllocations - 1);
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
		REQUIRE(allocator.GetFreeHandleCount() == MaxAllocations);
	}

	SECTION("Free and reallocate hands out a different slot")
	{
		Allocator allocator;
		uint32_t handle1 = allocator.Allocate();
		REQUIRE(handle1 == 0);					// index 0, generation 0
		allocator.Free(handle1);

		// FIFO: the freed index goes to the back of the queue, so the next allocation
		// takes the next untouched slot instead of handing index 0 straight back.
		// Under the old LIFO free list this returned 128 (index 0, generation 1).
		uint32_t handle2 = allocator.Allocate();
		REQUIRE(handle2 == 1);

		REQUIRE(allocator.GetUsedMemory() == TestStructSize);
		REQUIRE(allocator.GetCurrentAllocationCount() == 1);
		REQUIRE(allocator.GetTotalAllocationCount() == 2);
	}

	SECTION("A freed slot only returns after a full rotation")
	{
		Allocator allocator;
		uint32_t first = allocator.Allocate();
		REQUIRE(first == 0);
		allocator.Free(first);					// index 0 -> back of the queue, generation 1

		// Every other slot is handed out before index 0 comes back around.
		for (size_t i = 1; i < MaxAllocations; i++)
		{
			uint32_t handle = allocator.Allocate();
			REQUIRE(handle == static_cast<uint32_t>(i));	// still generation 0
		}

		// The queue now holds index 0 alone, carrying generation 1.
		uint32_t reused = allocator.Allocate();
		REQUIRE(reused == GenerationStep);
		REQUIRE(allocator.GetFreeHandleCount() == 0);
	}

	SECTION("Generation burn spreads across every slot")
	{
		// This is the property the FIFO free list exists for. Under LIFO this same
		// loop drove index 0 to generation 85 and left every other slot at 0.
		Allocator allocator;
		for (size_t i = 0; i < MaxAllocations; i++)
		{
			uint32_t handle = allocator.Allocate();
			REQUIRE(handle == static_cast<uint32_t>(i));	// generation 0 on the first pass
			allocator.Free(handle);
		}

		// One rotation later every slot sits at generation 1, none of them higher.
		for (size_t i = 0; i < MaxAllocations; i++)
		{
			uint32_t handle = allocator.Allocate();
			REQUIRE(handle == GenerationStep + static_cast<uint32_t>(i));
		}
		REQUIRE(allocator.GetFreeHandleCount() == 0);
	}

	SECTION("Reuse order survives the ring wrapping")
	{
		// MAX_BLOCK_COUNT is 85 here, not a power of two, so the wrap is a plain
		// compare rather than a mask. Filling the pool wraps the head; freeing the
		// whole pool afterwards wraps the tail.
		Allocator allocator;
		std::vector<uint32_t> live;
		live.reserve(MaxAllocations);
		for (size_t i = 0; i < MaxAllocations; i++)
			live.push_back(allocator.Allocate());

		REQUIRE(allocator.GetFreeHandleCount() == 0);

		for (uint32_t handle : live)
			allocator.Free(handle);

		REQUIRE(allocator.GetFreeHandleCount() == MaxAllocations);
		REQUIRE(allocator.GetCurrentAllocationCount() == 0);

		// Freed in index order, so they must come back in index order.
		for (size_t i = 0; i < MaxAllocations; i++)
			REQUIRE(allocator.Allocate() == GenerationStep + static_cast<uint32_t>(i));
	}

	SECTION("Allocate until full")
	{
		Allocator allocator;
		for (size_t i = 0; i < MaxAllocations; i++)
		{
			uint32_t handle = allocator.Allocate();
			TestStruct* ptr = allocator.GetPointerFromHandle(handle);
			ptr->a = static_cast<uint32_t>(i);
			REQUIRE(ptr != nullptr);

			TestStruct* ptr2 = allocator.GetPointerFromHandle(handle);
			REQUIRE(ptr2->a == static_cast<uint32_t>(i));
		}
		REQUIRE(allocator.GetUsedMemory() == MaxAllocations * TestStructSize);
		REQUIRE(allocator.GetCurrentAllocationCount() == MaxAllocations);
		REQUIRE(allocator.GetTotalAllocationCount() == MaxAllocations);

		// The exhaustion guard has to read the free count: m_FreeHandles is a
		// fixed-capacity ring now and its size() never drops to zero.
		REQUIRE(allocator.GetFreeHandleCount() == 0);
		REQUIRE_THROWS_AS(allocator.Allocate(), Substrate::AllocatorOutOfMemoryException);
	}

	SECTION("The top slot of a power-of-two pool is usable")
	{
		// Regression cover for GAP-030. The highest index equals the index mask here,
		// so a validity check that keys off "all index bits set" wrongly condemns it:
		// the slot allocates, but Free takes the retirement branch and it never
		// returns to the ring. Validity has to key off the sentinel instead.
		PowerOfTwoAllocator allocator;
		constexpr uint16_t topIndex = PowerOfTwoAllocator::MAX_BLOCK_COUNT - 1;	// 3 == index mask

		std::vector<uint16_t> handles;
		for (size_t i = 0; i < PowerOfTwoAllocator::MAX_BLOCK_COUNT; i++)
			handles.push_back(allocator.Allocate());

		REQUIRE(handles.back() == topIndex);				// generation 0, index 3
		REQUIRE(allocator.GetPointerFromHandle(handles.back()) != nullptr);
		REQUIRE(allocator.IsHandleValid(handles.back()));

		allocator.Free(handles.back());
		REQUIRE(allocator.GetMaxedGenerationCount() == 0);	// retirement must NOT have fired
		REQUIRE(allocator.GetFreeHandleCount() == 1);
		REQUIRE(allocator.GetCurrentAllocationCount() == PowerOfTwoAllocator::MAX_BLOCK_COUNT - 1);

		// It comes back with its generation incremented, like any other slot.
		constexpr uint16_t smallGenerationStep = 1u << PowerOfTwoAllocator::INDEX_BIT_COUNT;
		REQUIRE(allocator.Allocate() == static_cast<uint16_t>(smallGenerationStep + topIndex));
	}

	SECTION("Generation exhaustion retires a single slot")
	{
		// Hold every slot but one. With exactly one entry in the free list, FIFO
		// hands that same slot back every cycle, so its generation burns without
		// needing a full rotation per step.
		SmallAllocator allocator;
		constexpr size_t maxGenerations = (1u << SmallAllocator::GENERATION_BIT_COUNT) - 1;

		std::vector<uint16_t> held;
		for (size_t i = 1; i < SmallAllocator::MAX_BLOCK_COUNT; i++)
			held.push_back(allocator.Allocate());

		uint16_t cycling = allocator.Allocate();
		REQUIRE(allocator.GetFreeHandleCount() == 0);
		REQUIRE(allocator.GetCurrentAllocationCount() == SmallAllocator::MAX_BLOCK_COUNT);

		// Burn it up to the last usable generation. No REQUIRE inside the loop:
		// Catch2 assertions cost far more than the work being exercised.
		for (size_t generation = 1; generation < maxGenerations; generation++)
		{
			allocator.Free(cycling);
			cycling = allocator.Allocate();
		}
		REQUIRE(allocator.GetCurrentAllocationCount() == SmallAllocator::MAX_BLOCK_COUNT);

		// The next free saturates the generation. The slot is retired: it does not
		// return to the free list, so the pool is permanently one block short.
		allocator.Free(cycling);
		REQUIRE(allocator.GetFreeHandleCount() == 0);
		REQUIRE(allocator.GetMaxedGenerationCount() == 1);
		REQUIRE_THROWS_AS(allocator.Allocate(), Substrate::AllocatorOutOfMemoryException);

		// Known behaviour, not a fault in this test: a retired slot still counts as
		// allocated, so the count overstates from here on. GetMaxedGenerationCount()
		// is what makes that difference explainable rather than looking like a leak.
		REQUIRE(allocator.GetCurrentAllocationCount() == SmallAllocator::MAX_BLOCK_COUNT);
	}

	SECTION("Retiring one slot leaves the rest of the ring usable")
	{
		SmallAllocator allocator;
		constexpr size_t maxGenerations = (1u << SmallAllocator::GENERATION_BIT_COUNT) - 1;

		std::vector<uint16_t> held;
		for (size_t i = 1; i < SmallAllocator::MAX_BLOCK_COUNT; i++)
			held.push_back(allocator.Allocate());

		uint16_t cycling = allocator.Allocate();
		for (size_t generation = 1; generation < maxGenerations; generation++)
		{
			allocator.Free(cycling);
			cycling = allocator.Allocate();
		}
		allocator.Free(cycling);				// retires that slot

		// The slots held throughout still free and reallocate normally.
		for (uint16_t handle : held)
			allocator.Free(handle);

		REQUIRE(allocator.GetFreeHandleCount() == SmallAllocator::MAX_BLOCK_COUNT - 1);
		REQUIRE(allocator.GetMaxedGenerationCount() == 1);
		for (size_t i = 0; i < SmallAllocator::MAX_BLOCK_COUNT - 1; i++)
			REQUIRE(allocator.GetPointerFromHandle(allocator.Allocate()) != nullptr);

		REQUIRE_THROWS_AS(allocator.Allocate(), Substrate::AllocatorOutOfMemoryException);
	}
}
