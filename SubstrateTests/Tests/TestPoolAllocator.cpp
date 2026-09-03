#include <catch2/catch_test_macros.hpp>

#define SUBSTRATE_ENABLE_DETAILS
#include "Substrate/PoolAllocator.h"
#include "Substrate/UtilityFunctions.h"

#include <cstdint>
#include <vector>
#include <type_traits>


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

// 85 blocks needs 7 index bits, and 7 bits address 128 slots. Indices 85..127 are
// therefore representable in a handle but have no slot behind them. That gap is what
// the bounds check in GetInternalHandle exists for.
constexpr uint32_t IndexMask = (1u << Allocator::INDEX_BIT_COUNT) - 1;			// 127
constexpr uint32_t FirstUnbackedIndex = static_cast<uint32_t>(MaxAllocations);	// 85
static_assert(IndexMask > FirstUnbackedIndex);


// ---------------------------------------------------------------------------------
// Compile-time cover for HandleTypeCheck.
//
// It used to read `requires () { <expr>; }`, which only asks whether the expression
// is well-formed, never whether it is true -- so every type satisfied it, float and
// TestStruct included. These are the cases that silently passed before it became a
// plain conjunction of type traits.
// ---------------------------------------------------------------------------------
static_assert(Substrate::HandleTypeCheck<uint8_t>);
static_assert(Substrate::HandleTypeCheck<uint16_t>);
static_assert(Substrate::HandleTypeCheck<uint32_t>);
static_assert(Substrate::HandleTypeCheck<uint64_t>);

static_assert(!Substrate::HandleTypeCheck<int32_t>);		// signed
static_assert(!Substrate::HandleTypeCheck<signed char>);	// signed
static_assert(!Substrate::HandleTypeCheck<bool>);
static_assert(!Substrate::HandleTypeCheck<char>);			// signedness is implementation defined
static_assert(!Substrate::HandleTypeCheck<float>);
static_assert(!Substrate::HandleTypeCheck<double>);
static_assert(!Substrate::HandleTypeCheck<TestStruct>);


// ---------------------------------------------------------------------------------
// The invariants PoolAllocator now asserts on itself, restated for the pools this
// file configures. If one of them ever stops holding the allocator's own
// static_asserts fire first; these are here so the intent is readable next to the
// aliases that have to satisfy it. Being a power of two is deliberately not among
// them -- MAX_BLOCK_COUNT is 85 here and nothing in the allocator cares.
// ---------------------------------------------------------------------------------
static_assert(AllocatorSize >= sizeof(TestStruct));
static_assert(SmallAllocatorSize >= sizeof(TestStruct));
static_assert(PowerOfTwoAllocatorSize >= sizeof(TestStruct));

static_assert(Allocator::INDEX_BIT_COUNT < sizeof(uint32_t) * 8);
static_assert(SmallAllocator::INDEX_BIT_COUNT < sizeof(uint16_t) * 8);
static_assert(PowerOfTwoAllocator::INDEX_BIT_COUNT < sizeof(uint16_t) * 8);

static_assert(!Substrate::Utility::IsPowerOfTwo(Allocator::MAX_BLOCK_COUNT));


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

	SECTION("An index past the block count is rejected, not read")
	{
		// 7 index bits address 128 slots but only 85 exist. Without the bounds check in
		// GetInternalHandle these handles indexed m_Handles out of range -- a vector of
		// 85 entries read at [85..127].
		Allocator allocator;
		for (size_t i = 0; i < MaxAllocations; i++)
			allocator.Allocate();		// every real slot live, so a stray hit would look plausible

		// Generation 0, so the raw handle value is the index.
		for (uint32_t index = FirstUnbackedIndex; index <= IndexMask; index++)
		{
			REQUIRE(allocator.IsHandleValid(index) == false);
			REQUIRE(allocator.GetPointerFromHandle(index) == nullptr);
		}

		// Free has to drop them just as quietly: no ring entry, no counter movement.
		allocator.Free(FirstUnbackedIndex);
		allocator.Free(IndexMask);
		REQUIRE(allocator.GetFreeHandleCount() == 0);
		REQUIRE(allocator.GetCurrentAllocationCount() == MaxAllocations);
		REQUIRE(allocator.GetMaxedGenerationCount() == 0);
	}

	SECTION("The all-ones sentinel is rejected by every entry point")
	{
		// A default-constructed Aurora ResourceHandle carries exactly this value, so the
		// sentinel reaches the allocator through ordinary code, not just tampering.
		Allocator allocator;
		constexpr uint32_t sentinel = Allocator::InternalHandle::INVALID_HANDLE;
		REQUIRE(sentinel > IndexMask);

		allocator.Allocate();

		REQUIRE(allocator.IsHandleValid(sentinel) == false);
		REQUIRE(allocator.GetPointerFromHandle(sentinel) == nullptr);

		allocator.Free(sentinel);
		REQUIRE(allocator.GetFreeHandleCount() == MaxAllocations - 1);
		REQUIRE(allocator.GetCurrentAllocationCount() == 1);
		REQUIRE(allocator.GetMaxedGenerationCount() == 0);

		// On a pool whose block count is a power of two the sentinel's index bits name a
		// real slot, so the bounds check cannot fire -- the raw compare in Equals is what
		// rejects it there. Both pool shapes have to refuse it.
		PowerOfTwoAllocator powerOfTwo;
		constexpr uint16_t smallSentinel = PowerOfTwoAllocator::InternalHandle::INVALID_HANDLE;
		constexpr uint16_t smallIndexMask = (1u << PowerOfTwoAllocator::INDEX_BIT_COUNT) - 1;
		static_assert(smallIndexMask < PowerOfTwoAllocator::MAX_BLOCK_COUNT);

		REQUIRE(powerOfTwo.IsHandleValid(smallSentinel) == false);
		REQUIRE(powerOfTwo.GetPointerFromHandle(smallSentinel) == nullptr);
	}

	SECTION("A freed handle stops resolving")
	{
		// The use-after-free case: without the Equals check in the lookup a freed handle
		// still produced a live pointer into the block that was handed back to the ring.
		Allocator allocator;
		uint32_t handle = allocator.Allocate();
		REQUIRE(allocator.GetPointerFromHandle(handle) != nullptr);
		REQUIRE(allocator.IsHandleValid(handle));

		allocator.Free(handle);
		REQUIRE(allocator.IsHandleValid(handle) == false);
		REQUIRE(allocator.GetPointerFromHandle(handle) == nullptr);

		// And a double free must not credit the ring a second time.
		allocator.Free(handle);
		REQUIRE(allocator.GetFreeHandleCount() == MaxAllocations);
		REQUIRE(allocator.GetCurrentAllocationCount() == 0);
		REQUIRE(allocator.GetMaxedGenerationCount() == 0);
	}

	SECTION("A stale handle stays stale once its slot is reused")
	{
		Allocator allocator;
		uint32_t stale = allocator.Allocate();			// index 0, generation 0
		allocator.Free(stale);

		// Drain the ring so index 0 comes back around carrying generation 1.
		for (size_t i = 1; i < MaxAllocations; i++)
			allocator.Allocate();

		uint32_t reused = allocator.Allocate();
		REQUIRE(reused == GenerationStep);				// index 0 again, one generation on
		REQUIRE(reused != stale);

		// Same slot, same index bits, older generation. Only the generation separates them.
		REQUIRE(allocator.GetPointerFromHandle(reused) != nullptr);
		REQUIRE(allocator.GetPointerFromHandle(stale) == nullptr);
		REQUIRE(allocator.IsHandleValid(stale) == false);

		// Freeing the stale handle must not release the slot its index names.
		allocator.Free(stale);
		REQUIRE(allocator.GetFreeHandleCount() == 0);
		REQUIRE(allocator.IsHandleValid(reused));
	}

	SECTION("The const lookup resolves to the same block")
	{
		Allocator allocator;
		uint32_t handle = allocator.Allocate();

		TestStruct* mutablePtr = allocator.GetPointerFromHandle(handle);
		REQUIRE(mutablePtr != nullptr);
		mutablePtr->a = 7;

		const Allocator& constAllocator = allocator;
		static_assert(std::is_same_v<decltype(constAllocator.GetPointerFromHandle(handle)), const TestStruct*>);
		static_assert(std::is_same_v<decltype(allocator.GetPointerFromHandle(handle)), TestStruct*>);

		// The non-const overload forwards to the const one and casts the result back, so
		// the two must agree on the address and on every rejection.
		const TestStruct* constPtr = constAllocator.GetPointerFromHandle(handle);
		REQUIRE(constPtr == mutablePtr);
		REQUIRE(constPtr->a == 7);
		REQUIRE(constAllocator.IsHandleValid(handle));
		REQUIRE(constAllocator.GetPointerFromHandle(Allocator::InternalHandle::INVALID_HANDLE) == nullptr);

		allocator.Free(handle);
		REQUIRE(constAllocator.GetPointerFromHandle(handle) == nullptr);
	}

	SECTION("Freeing an already-retired slot does not inflate the retirement count")
	{
		// The retirement counter is what makes a shrinking pool explainable rather than
		// looking like a leak, so a repeat Free of a retired slot must not keep bumping it.
		// This is only reachable on a pool whose top index equals the index mask: the
		// retired entry is then bit-for-bit the all-ones sentinel, and the entry matches
		// on Equals while IncrementGeneration leaves it unchanged.
		PowerOfTwoAllocator allocator;
		constexpr size_t maxGenerations = (1u << PowerOfTwoAllocator::GENERATION_BIT_COUNT) - 1;
		constexpr uint16_t sentinel = PowerOfTwoAllocator::InternalHandle::INVALID_HANDLE;
		constexpr uint16_t generationStep = 1u << PowerOfTwoAllocator::INDEX_BIT_COUNT;

		std::vector<uint16_t> held;
		for (size_t i = 1; i < PowerOfTwoAllocator::MAX_BLOCK_COUNT; i++)
			held.push_back(allocator.Allocate());

		uint16_t cycling = allocator.Allocate();
		REQUIRE(cycling == PowerOfTwoAllocator::MAX_BLOCK_COUNT - 1);	// index 3 == the index mask

		for (size_t generation = 1; generation < maxGenerations; generation++)
		{
			allocator.Free(cycling);
			cycling = allocator.Allocate();
		}
		REQUIRE(cycling == static_cast<uint16_t>(sentinel - generationStep));	// one generation short

		allocator.Free(cycling);										// saturates -> slot retired
		REQUIRE(allocator.GetMaxedGenerationCount() == 1);
		REQUIRE(allocator.GetFreeHandleCount() == 0);

		// m_Handles[3] now equals the sentinel exactly. Each of these used to take the
		// retirement branch and increment the counter again.
		for (int i = 0; i < 5; i++)
			allocator.Free(sentinel);

		REQUIRE(allocator.GetMaxedGenerationCount() == 1);
		REQUIRE(allocator.GetFreeHandleCount() == 0);
		REQUIRE(allocator.IsHandleValid(sentinel) == false);
		REQUIRE(allocator.GetPointerFromHandle(sentinel) == nullptr);

		// The handle that was live one generation ago is dead too, by generation mismatch.
		REQUIRE(allocator.IsHandleValid(cycling) == false);
	}

	SECTION("Reset recycles the pool instead of tearing it down")
	{
		// Reset used to free() the block, null it and zero m_TotalSize -- the allocator was
		// dead afterwards and Allocate threw. It is a recycle now: the block stays, the
		// destructor owns the free, every slot returns to generation 0 and the ring refills.
		Allocator allocator;
		for (size_t i = 0; i < MaxAllocations; i++)
			allocator.Allocate();
		REQUIRE(allocator.GetFreeHandleCount() == 0);

		allocator.Reset();

		REQUIRE(allocator.GetTotalMemory() == AllocatorSize);
		REQUIRE(allocator.GetFreeHandleCount() == MaxAllocations);
		REQUIRE(allocator.GetCurrentAllocationCount() == 0);
		REQUIRE(allocator.GetMaxedGenerationCount() == 0);

		// The lifetime counter is cleared too, not carried across the reset.
		REQUIRE(allocator.GetTotalAllocationCount() == 0);

		// Usable again, and back to handing out index 0 at generation 0.
		uint32_t handle = allocator.Allocate();
		REQUIRE(handle == 0);
		REQUIRE(allocator.GetPointerFromHandle(handle) != nullptr);
	}

	SECTION("Reset revives handles issued before it")
	{
		// Known behaviour, and the reason Reset is a scene-boundary operation rather than a
		// general-purpose one: generations go back to 0, so a handle from before the reset
		// compares equal to the fresh slot and resolves again. Nothing detects the reuse --
		// every outstanding handle has to be dead before Reset is called.
		Allocator allocator;
		uint32_t before = allocator.Allocate();		// index 0, generation 0
		allocator.Free(before);						// index 0 moves to generation 1
		REQUIRE(allocator.IsHandleValid(before) == false);

		allocator.Reset();
		REQUIRE(allocator.IsHandleValid(before));	// generation 0 again -> resolves
	}

	SECTION("Reset returns a retired slot to service")
	{
		// Generation exhaustion is permanent for the life of the pool; Reset is the only
		// thing that undoes it. This is the recycle path scene loading needs.
		PowerOfTwoAllocator allocator;
		constexpr size_t maxGenerations = (1u << PowerOfTwoAllocator::GENERATION_BIT_COUNT) - 1;

		for (size_t i = 1; i < PowerOfTwoAllocator::MAX_BLOCK_COUNT; i++)
			allocator.Allocate();

		uint16_t cycling = allocator.Allocate();
		for (size_t generation = 1; generation < maxGenerations; generation++)
		{
			allocator.Free(cycling);
			cycling = allocator.Allocate();
		}
		allocator.Free(cycling);					// retires the slot
		REQUIRE(allocator.GetMaxedGenerationCount() == 1);
		REQUIRE(allocator.GetFreeHandleCount() == 0);

		allocator.Reset();

		REQUIRE(allocator.GetMaxedGenerationCount() == 0);
		REQUIRE(allocator.GetFreeHandleCount() == PowerOfTwoAllocator::MAX_BLOCK_COUNT);
		for (size_t i = 0; i < PowerOfTwoAllocator::MAX_BLOCK_COUNT; i++)
			REQUIRE(allocator.GetPointerFromHandle(allocator.Allocate()) != nullptr);
	}
}
