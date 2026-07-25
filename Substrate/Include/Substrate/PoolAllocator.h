#pragma once
#include "Substrate/AllocatorBase.h"
#include "Substrate/BaseHandle.h"
#include "Substrate/Defines.h"
#include "Substrate/Exceptions.h"
#include "Substrate/UtilityFunctions.h"

#include <vector>

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

		const std::vector<uint32_t>& GetFreeHandleIndices() const { return m_FreeHandles; }
		using InternalHandle = DefineHandle<GENERATION_BIT_COUNT, INDEX_BIT_COUNT, TResourceHandle>;
		const std::vector<InternalHandle>& GetHandles() const { return m_Handles; }

#ifdef SUBSTRATE_DETAILS_ENABLED
		inline size_t GetTotalMemory()				const override { return m_TotalSize; }
		inline size_t GetMaxAllocationCount()		const override { return MAX_BLOCK_COUNT; }
		inline size_t GetUsedMemory()				const override { return m_CurrentAllocationCount * sizeof(TBlockType); }
		inline size_t GetCurrentAllocationCount()	const override { return m_CurrentAllocationCount; }
		inline size_t GetTotalAllocationCount()		const override { return m_TotalAllocationCount; }
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
	};


	template<typename TBlockType, typename TResourceHandle, size_t TSize>
		requires HandleTypeCheck<TResourceHandle> && MustBePowerOFTwo<TSize>
	PoolAllocator<TBlockType, TResourceHandle, TSize>::PoolAllocator()
		: m_TotalSize(TSize)
	{
		m_MemoryBlock = static_cast<TBlockType*>(malloc(TSize));
		m_Handles.reserve(MAX_BLOCK_COUNT);
		m_FreeHandles.reserve(MAX_BLOCK_COUNT);

		// Initialize handles in reverse order for better cache locality
		for (uint32_t i = 0; i < MAX_BLOCK_COUNT; i++)
		{
			m_Handles.push_back(InternalHandle(i));
			m_FreeHandles.push_back(MAX_BLOCK_COUNT-1-i);
		}
	}

	template<typename TBlockType, typename TResourceHandle, size_t TSize>
		requires HandleTypeCheck<TResourceHandle> && MustBePowerOFTwo<TSize>
	PoolAllocator<TBlockType, TResourceHandle, TSize>::~PoolAllocator()
	{
		Reset();
	}

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
	}

	template<typename TBlockType, typename TResourceHandle, size_t TSize>
		requires HandleTypeCheck<TResourceHandle> && MustBePowerOFTwo<TSize>
	TResourceHandle PoolAllocator<TBlockType, TResourceHandle, TSize>::Allocate()
	{
		// Check if there are free handles
		if(m_FreeHandles.size() == 0)
			throw AllocatorOutOfMemoryException("Pool allocator out of memory");

		// Get the next free handle
		uint32_t idx = m_FreeHandles.back();
		m_FreeHandles.pop_back();
		m_CurrentAllocationCount++;
		m_TotalAllocationCount++;
		return m_Handles[idx].GetRaw();
	}

	template<typename TBlockType, typename TResourceHandle, size_t TSize>
		requires HandleTypeCheck<TResourceHandle>&& MustBePowerOFTwo<TSize>
	bool PoolAllocator<TBlockType, TResourceHandle, TSize>::IsHandleValid(TResourceHandle resourceHandle)
	{
		InternalHandle handle = InternalHandle::FromRawType(resourceHandle);
		InternalHandle& internal = m_Handles[handle.Index()];
		if (!internal.Equals(handle))
			return false;
		return true;
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
		if (!internal.Equals(handle))
			return; // Invalid handle or handle was already freed

		internal = internal.IncrementGeneration();

		// Check if generation is maxed out
		if(!internal.IsValid())
			return; // Cannot free handle anymore

		m_FreeHandles.push_back(internal.Index());

		// Decrease allocation count only if handle was put back into the free list
		m_CurrentAllocationCount--;
	}

}
