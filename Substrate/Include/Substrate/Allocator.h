#pragma once

namespace Substrate {

	struct AllocatorSpecification
	{
		size_t Size = 0;
		size_t Alignment = 8;
	};

	/// <summary>
	/// This is just an interface. The memory management logic will be implemented in derived classes.
	/// </summary>
	class BaseAllocator
	{
	public:
		virtual ~BaseAllocator() = default;

		virtual void Create(const AllocatorSpecification& spec) = 0;
		virtual void Destroy() = 0;

		virtual void* Allocate(size_t size, size_t alignment = 8) = 0;
		virtual void Free(void* pointer) = 0;
	};
}


// Allocators:
// - LinearAllocator (StackAllocator) -> nextAddress is always growing, except when last allocation deallocates (nice for LIFO, like a stack)
// - PoolAllocator -> fixed-size objects, preallocated pool of memory blocks (nice for frequently allocated/deallocated objects of the same size), destroyed objects just mark a block as free for reuse (in a free list for example)
// - ArenaAllocator -> multiple objects in one big preallocated chunk of memory, then deallocate the whole arena at once instead of each individual object (e.g. local variables in a scope, like stack frame of a function)
