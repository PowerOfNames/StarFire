#pragma once
#include "Substrate/AllocatorBase.h"
#include "Substrate/BaseHandle.h"
#include "Substrate/Defines.h"
#include "Substrate/Exceptions.h"
#include "Substrate/UtilityFunctions.h"

#include <vector>
#include <new>
#include <type_traits>
#include <cstddef>


#define PA_TEMPLATE template<typename TBlockType, HandleTypeCheck TResourceHandle, size_t TSize>
#define PA_CLASS PoolAllocator<TBlockType, TResourceHandle, TSize>

namespace Substrate {

	PA_TEMPLATE
	class PoolAllocator : public AllocatorBase
	{
		static_assert(TSize >= sizeof(TBlockType), "PoolAllocator size must be at least the size of one block.");

	public:
		static constexpr uint32_t MAX_BLOCK_COUNT = static_cast<uint32_t>(TSize / sizeof(TBlockType));		// Maximum number of blocks that can be allocated in the pool
		static constexpr uint8_t INDEX_BIT_COUNT = Utility::Log2Up(TSize / sizeof(TBlockType));				// Number of bits needed to represent the maximum block count (e.g., 7 bits for 128 blocks)
		static constexpr uint8_t GENERATION_BIT_COUNT = (sizeof(TResourceHandle) * 8) - INDEX_BIT_COUNT;	// Remaining bits for generation count (e.g., 25 bits for a 32-bit handle with 7 bits for index) 
																											// -> 25 bits for generation allows for 33,554,432 generations before handle reuse becomes unsafe
		static_assert(INDEX_BIT_COUNT < (sizeof(TResourceHandle) * 8), "PoolAllocator index bit count is too large for the given handle type.");


		PoolAllocator();
		~PoolAllocator();

		void Reset();

		TResourceHandle Allocate();
		const TBlockType* GetPointerFromHandle(TResourceHandle handle) const;
		TBlockType* GetPointerFromHandle(TResourceHandle handle);
		bool IsHandleValid(TResourceHandle handle) const;
		void Free(TResourceHandle handle);

		const uint32_t GetFreeHandleCount() const { return m_FreeCount; }
		using InternalHandle = DefineHandle<GENERATION_BIT_COUNT, INDEX_BIT_COUNT, TResourceHandle>;
		const std::vector<InternalHandle>& GetHandles() const { return m_Handles; }

#ifdef SUBSTRATE_DETAILS_ENABLED
		inline size_t GetTotalMemory()				const override { return m_TotalSize; }
		inline size_t GetMaxAllocationCount()		const override { return MAX_BLOCK_COUNT; }
		inline size_t GetUsedMemory()				const override { return m_CurrentAllocationCount * sizeof(TBlockType); }
		inline size_t GetCurrentAllocationCount()	const override { return m_CurrentAllocationCount; }
		inline size_t GetTotalAllocationCount()		const override { return m_TotalAllocationCount; }
		inline size_t GetMaxedGenerationCount()		const override { return m_MaxedGenerationCount; }
		inline size_t GetResetCount()				const override { return 0; }
#endif
	private:
		const InternalHandle* GetInternalHandle(TResourceHandle handle) const;
		InternalHandle* GetInternalHandle(TResourceHandle handle);

	private:
		static constexpr TResourceHandle INVALID_HANDLE = InternalHandle::INVALID_HANDLE;

		TBlockType* m_MemoryBlock = nullptr;
		size_t m_TotalSize;
		std::vector<InternalHandle> m_Handles;
		std::vector<uint32_t> m_FreeHandles;
		uint32_t m_FreeHead = 0;
		uint32_t m_FreeTail = 0;
		uint32_t m_FreeCount = 0;

		size_t m_CurrentAllocationCount = 0;
		size_t m_TotalAllocationCount = 0;
		uint32_t m_MaxedGenerationCount = 0;
	};


	PA_TEMPLATE
	PA_CLASS::PoolAllocator()
		: m_TotalSize(TSize)
	{
		//guards every BlockType that is not 16 byte-aligned -> if this happens, allocator needs to be upgraded
		static_assert(alignof(TBlockType) <= alignof(std::max_align_t), "PoolAllocator currently only supports default 16 byte alignment. Please upgrade implmentation for larger alignements.");
		static_assert(std::is_trivially_destructible_v<TBlockType>,	"PoolAllocator slots are data-only: Free() does not run destructors.");

		m_MemoryBlock = static_cast<TBlockType*>(malloc(TSize));
		m_Handles.reserve(MAX_BLOCK_COUNT);
		m_FreeHandles.reserve(MAX_BLOCK_COUNT);
		
		Reset();
	}

	PA_TEMPLATE
	PA_CLASS::~PoolAllocator()
	{
		Reset();
		if (!m_MemoryBlock)
			return;

		free(m_MemoryBlock);
		m_MemoryBlock = nullptr;
		m_TotalSize = 0;
	}

	
	PA_TEMPLATE
	void PA_CLASS::Reset()
	{	
		if (!m_MemoryBlock)
			return;
		m_Handles.clear();
		m_FreeHandles.clear();
		m_FreeCount = MAX_BLOCK_COUNT;
		m_FreeHead = 0;
		m_FreeTail = 0;

		m_CurrentAllocationCount = 0;
		m_TotalAllocationCount = 0;
		m_MaxedGenerationCount = 0;

		for (uint32_t i = 0; i < MAX_BLOCK_COUNT; i++)
		{
			m_Handles.push_back(InternalHandle(i));
			m_FreeHandles.push_back(i);
		}
	}

	PA_TEMPLATE
	TResourceHandle PA_CLASS::Allocate()
	{
		// Check if there are free handles
		if(m_FreeCount == 0)
			throw AllocatorOutOfMemoryException("Pool allocator out of memory");

		// Get the next free handle
		uint32_t idx = m_FreeHandles[m_FreeHead];
		m_FreeHead = (m_FreeHead + 1) % MAX_BLOCK_COUNT;
		//Calls constructor -> sets default values
		new(m_MemoryBlock + idx) TBlockType{};		

		m_CurrentAllocationCount++;
		m_TotalAllocationCount++;
		m_FreeCount--;
		return m_Handles[idx].GetRaw();
	}

	PA_TEMPLATE
	bool PA_CLASS::IsHandleValid(TResourceHandle resourceHandle) const
	{
		return GetInternalHandle(resourceHandle) != nullptr;
	}

	PA_TEMPLATE
	const TBlockType* PA_CLASS::GetPointerFromHandle(TResourceHandle resourceHandle) const
	{
		// Validate handle
		const InternalHandle* internal = GetInternalHandle(resourceHandle);
		if (!internal)
			return nullptr; // Invalid handle or handle was already freed or generation mismatch

		const TBlockType* blockPtr = m_MemoryBlock + internal->Index();
		return blockPtr;
	}

	PA_TEMPLATE
	TBlockType* PA_CLASS::GetPointerFromHandle(TResourceHandle resourceHandle)
	{
		return const_cast<TBlockType*>(std::as_const(*this).GetPointerFromHandle(resourceHandle));
	}


	PA_TEMPLATE
	void PA_CLASS::Free(TResourceHandle resourceHandle)
	{
		// Validate handle
		InternalHandle* internal = GetInternalHandle(resourceHandle);
		if (!internal)
			return; // Invalid handle or handle was already freed

		*internal = internal->IncrementGeneration();

		// Check if generation is maxed out
		if (!internal->IsValid())
		{
			m_MaxedGenerationCount++;
			return; // Cannot free handle anymore
		}

		m_FreeHandles[m_FreeTail] = internal->Index();
		m_FreeCount++;
		m_FreeTail = (m_FreeTail + 1) % MAX_BLOCK_COUNT;

		// Decrease allocation count only if handle was put back into the free list
		m_CurrentAllocationCount--;
	}

	PA_TEMPLATE
	const PA_CLASS::InternalHandle* PA_CLASS::GetInternalHandle(TResourceHandle handle) const
	{
		InternalHandle internalHandle = InternalHandle::FromRawType(handle);
		TResourceHandle index = internalHandle.Index();
		if (index >= MAX_BLOCK_COUNT)
			return nullptr; // Invalid index

		const InternalHandle& internal = m_Handles[index];
		if (!internal.Equals(internalHandle) || !internal.IsValid())
			return nullptr; // Invalid handle or handle was already freed
		return &internal;
	}
	
	PA_TEMPLATE
	PA_CLASS::InternalHandle* PA_CLASS::GetInternalHandle(TResourceHandle handle)
	{
		return const_cast<InternalHandle*>(std::as_const(*this).GetInternalHandle(handle));
	}
}

#undef PA_TEMPLATE
#undef PA_CLASS
