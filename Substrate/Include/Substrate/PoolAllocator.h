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

namespace Substrate {

	template<size_t TSize>
	concept MustBePowerOFTwo = requires ()
	{
		Utility::IsPowerOfTwo(TSize);
	};

	template<typename TBlockType, typename TResourceHandle, size_t TSize>
		requires HandleTypeCheck<TResourceHandle> && MustBePowerOFTwo<TSize>
	class PoolAllocator : public AllocatorBase
	{
	public:
		static constexpr uint32_t MAX_BLOCK_COUNT = static_cast<uint32_t>(TSize / sizeof(TBlockType));		// Maximum number of blocks that can be allocated in the pool
		static constexpr uint8_t INDEX_BIT_COUNT = Utility::Log2Up(TSize / sizeof(TBlockType));				// Number of bits needed to represent the maximum block count (e.g., 7 bits for 128 blocks)
		static constexpr uint8_t GENERATION_BIT_COUNT = (sizeof(TResourceHandle) * 8) - INDEX_BIT_COUNT;	// Remaining bits for generation count (e.g., 25 bits for a 32-bit handle with 7 bits for index) 
																											// -> 25 bits for generation allows for 33,554,432 generations before handle reuse becomes unsafe

		PoolAllocator();
		~PoolAllocator();

		void Reset();

		TResourceHandle Allocate();
		TBlockType* GetPointerFromHandle(TResourceHandle handle);
		bool IsHandleValid(TResourceHandle handle);
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
		static constexpr TResourceHandle INVALID_HANDLE = InternalHandle::INVALID_HANDLE;

		TBlockType* m_MemoryBlock = nullptr;
		size_t m_TotalSize;
		size_t m_CurrentAllocationCount = 0;
		size_t m_TotalAllocationCount = 0;
		std::vector<InternalHandle> m_Handles;
		std::vector<uint32_t> m_FreeHandles;
		uint32_t m_FreeHead = 0;
		uint32_t m_FreeTail = 0;
		uint32_t m_FreeCount = 0;
		uint32_t m_MaxedGenerationCount = 0;
	};


	template<typename TBlockType, typename TResourceHandle, size_t TSize>
		requires HandleTypeCheck<TResourceHandle> && MustBePowerOFTwo<TSize>
	PoolAllocator<TBlockType, TResourceHandle, TSize>::PoolAllocator()
		: m_TotalSize(TSize)
	{
		//guards every BlockType that is not 16 byte-aligned -> if this happens, allocator needs to be upgraded
		static_assert(alignof(TBlockType) <= alignof(std::max_align_t), "PoolAllocator currently only supports default 16 byte alignment. Please upgrade implmentation for larger alignements.");
		static_assert(std::is_trivially_destructible_v<TBlockType>,	"PoolAllocator slots are data-only: Free() does not run destructors.");

		m_MemoryBlock = static_cast<TBlockType*>(malloc(TSize));
		m_Handles.reserve(MAX_BLOCK_COUNT);
		m_FreeHandles.reserve(MAX_BLOCK_COUNT);

		m_FreeCount = MAX_BLOCK_COUNT;
		m_FreeHead = 0;
		m_FreeTail = 0;

		// Initialize handles in reverse order for better cache locality
		for (uint32_t i = 0; i < MAX_BLOCK_COUNT; i++)
		{
			m_Handles.push_back(InternalHandle(i));
			m_FreeHandles.push_back(i);
		}
	}

	template<typename TBlockType, typename TResourceHandle, size_t TSize>
		requires HandleTypeCheck<TResourceHandle> && MustBePowerOFTwo<TSize>
	PoolAllocator<TBlockType, TResourceHandle, TSize>::~PoolAllocator()
	{
		Reset();
	}

	/// <summary>
	/// Reset does NOT call any destructors! Allocator is for data only.
	/// </summary>
	/// <typeparam name="TBlockType"></typeparam>
	/// <typeparam name="TResourceHandle"></typeparam>
	/// <typeparam name="TSize"></typeparam>
	template<typename TBlockType, typename TResourceHandle, size_t TSize>
		requires HandleTypeCheck<TResourceHandle>&& MustBePowerOFTwo<TSize>
	void PoolAllocator<TBlockType, TResourceHandle, TSize>::Reset()
	{
		if (!m_MemoryBlock)
			return;

		free(m_MemoryBlock);
		m_MemoryBlock = nullptr;
		m_TotalSize = 0;
		m_Handles.clear();
		m_FreeHandles.clear();
		m_FreeHead = 0;
		m_FreeTail = 0;
		//Currently blocks further usage of the Allocator
		m_FreeCount = 0;
	}

	template<typename TBlockType, typename TResourceHandle, size_t TSize>
		requires HandleTypeCheck<TResourceHandle> && MustBePowerOFTwo<TSize>
	TResourceHandle PoolAllocator<TBlockType, TResourceHandle, TSize>::Allocate()
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

	template<typename TBlockType, typename TResourceHandle, size_t TSize>
		requires HandleTypeCheck<TResourceHandle>&& MustBePowerOFTwo<TSize>
	bool PoolAllocator<TBlockType, TResourceHandle, TSize>::IsHandleValid(TResourceHandle resourceHandle)
	{
		InternalHandle handle = InternalHandle::FromRawType(resourceHandle);
		InternalHandle& internal = m_Handles[handle.Index()];
		if (internal.IsValid() && internal.Equals(handle))
			return true;
		return false;
	}

	template<typename TBlockType, typename TResourceHandle, size_t TSize>
		requires HandleTypeCheck<TResourceHandle> && MustBePowerOFTwo<TSize>
	TBlockType* PoolAllocator<TBlockType, TResourceHandle, TSize>::GetPointerFromHandle(TResourceHandle resourceHandle)
	{
		// Validate handle
		InternalHandle handle = InternalHandle::FromRawType(resourceHandle);
		InternalHandle& internal = m_Handles[handle.Index()];
		if (!IsHandleValid(resourceHandle))
			return nullptr; // Invalid handle or handle was already freed

		TBlockType* blockPtr = m_MemoryBlock + internal.Index();
		return blockPtr;
	}

	template<typename TBlockType, typename TResourceHandle, size_t TSize>
		requires HandleTypeCheck<TResourceHandle> && MustBePowerOFTwo<TSize>
	void PoolAllocator<TBlockType, TResourceHandle, TSize>::Free(TResourceHandle resourceHandle)
	{
		// Validate handle
		InternalHandle handle = InternalHandle::FromRawType(resourceHandle);
		InternalHandle& internal = m_Handles[handle.Index()];
		if (!internal.Equals(handle) || !internal.IsValid())
			return; // Invalid handle or handle was already freed

		internal = internal.IncrementGeneration();

		// Check if generation is maxed out
		if (!internal.IsValid())
		{
			m_MaxedGenerationCount++;
			return; // Cannot free handle anymore
		}

		m_FreeHandles[m_FreeTail] = internal.Index();
		m_FreeCount++;
		m_FreeTail = (m_FreeTail + 1) % MAX_BLOCK_COUNT;

		// Decrease allocation count only if handle was put back into the free list
		m_CurrentAllocationCount--;
	}

}
