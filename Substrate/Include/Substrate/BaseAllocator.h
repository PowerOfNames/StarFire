#pragma once
#include "Substrate/Defines.h"
#include "Substrate/BaseHandle.h"

#include <concepts>
#include <type_traits>
namespace Substrate {

	struct AllocatorSpecification
	{
		void* MemoryBlock = nullptr;
		
		/// <summary>
		/// In bytes
		/// </summary>
		size_t Size = 0;
		size_t MinimumObjectCount = 0;

		///<summary>
		/// In bytes. Will be clamped to 1 to alignof(std::max_align_t)
		/// </summary>
		uint16_t Alignment = 1;
	};

	/// <summary>
	/// This is just an interface. The memory management logic will be implemented in derived classes.
	/// </summary>
	template<typename THandleType>
		requires HandleTypeCheck<THandleType>
	class BaseAllocator
	{
	public:
		virtual ~BaseAllocator() = default;

		virtual void Destroy() = 0;

		virtual THandleType Allocate(size_t size, size_t alignment = 8) = 0;
		virtual void* GetPointerFromHandle(THandleType handle) = 0;

#ifdef SUBSTRATE_DETAILS_ENABLED
		virtual size_t GetUsedMemory() const = 0;
		virtual size_t GetTotalMemory() const = 0;
		virtual size_t GetAllocationCount() const = 0;
#endif
	};
}


// Allocators:
// - Linear(Stack)Allocator -> nextAddress is always growing, except when last allocation deallocates (nice for LIFO, like a stack)
// - PoolAllocator			-> fixed-size objects, preallocated pool of memory blocks (nice for frequently allocated/deallocated objects of the same size), destroyed objects just mark a block as free for reuse (in a free list for example)
// - FreeListAllocator		-> variable-size objects, keeps track of free memory blocks and merges adjacent free blocks to reduce fragmentation (nice for general-purpose allocation with varying sizes)
// - ArenaAllocator			-> multiple objects in one big preallocated chunk of memory, then deallocate the whole arena at once instead of each individual object (e.g. local variables in a scope, like stack frame of a function)

// - BuddyAllocator			-> variable-size objects, splits memory into power-of-two sized blocks and merges them back when freed (nice for reducing fragmentation while allowing variable sizes)
// - SlabAllocator			-> fixed-size objects, maintains multiple pools (slabs) for different object sizes (nice for kernel memory management, where objects of certain sizes are frequently allocated/deallocated)
// - MonotonicAllocator		-> similar to Linear Allocator, but never frees individual allocations, only frees all at once (nice for temporary allocations that last for the lifetime of a frame or a scope)
// - VirtualMemoryAllocator -> uses OS-level virtual memory management for large allocations, allowing for on-demand paging and protection (nice for applications requiring large contiguous memory blocks e.g. streaming worlds)
// - DoubleEndedAllocator	-> allows allocations from both ends of a memory block, useful for scenarios where two different allocation patterns are needed (e.g. front for temporary data, back for long-lived data)