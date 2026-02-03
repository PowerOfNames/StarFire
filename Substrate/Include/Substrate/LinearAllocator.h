#pragma once
#include "Substrate/AllocatorBase.h"
#include "Substrate/Defines.h"
#include "Substrate/Exceptions.h"
#include "Substrate/UtilityFunctions.h"

#include <cstdlib>
#include <sstream>

namespace Substrate {


	template<typename TBlockType, size_t TSize>
	class LinearAllocator : public AllocatorBase
	{
	public:
		static constexpr uint32_t MAX_BLOCK_COUNT = static_cast<uint32_t>(TSize / sizeof(TBlockType));
		
		LinearAllocator();
		~LinearAllocator();

		TBlockType* Allocate(uint32_t count = 1);	
		struct ArrayView
		{
			TBlockType* Data;
			size_t Count;

			TBlockType& operator[](size_t index)
			{
				if (index > Count)
				{
					std::ostringstream oss;
					oss << "Index " << index << " out of scope (" << Count << ")";
					throw ArrayIndexOutOfBoundsException(oss.str().c_str());
				}
				return Data[index];
			}
			const TBlockType& operator[](size_t index) const
			{
				if (index > Count)
				{
					std::ostringstream oss;
					oss << "Index " << index << " out of scope (" << Count << ")";
					throw ArrayIndexOutOfBoundsException(oss.str().c_str());
				}
				return Data[index];
			}
		};
		ArrayView AllocateArray(uint32_t count)
		{
			TBlockType* data = Allocate(count);
			if (!data)
				return { nullptr, 0 };
			return { data, count };
		}
		void Reset();

#ifdef SUBSTRATE_DETAILS_ENABLED
		inline size_t GetUsedMemory()				const override { return static_cast<size_t>(m_CurrentPtr - m_MemoryBlock); }
		inline size_t GetMaxAllocationCount()		const override { return MAX_BLOCK_COUNT; }
		inline size_t GetTotalMemory()				const override { return m_TotalSize; }
		inline size_t GetTotalAllocationCount()		const override { return m_TotalAllocationCount; }
		inline size_t GetCurrentAllocationCount()	const override { return m_CurrentAllocationCount; }
		inline size_t GetResetCount()				const override { return m_ResetCount; }
#endif
	private:
		size_t m_TotalSize = 0;
		TBlockType* m_MemoryBlock = nullptr;
		TBlockType* m_CurrentPtr = nullptr;

#ifdef SUBSTRATE_DETAILS_ENABLED
		size_t m_CurrentAllocationCount = 0;
		size_t m_TotalAllocationCount = 0;
		size_t m_ResetCount = 0;
#endif
	};

	template<typename TBlockType, size_t TSize>
	LinearAllocator<TBlockType, TSize>::LinearAllocator()
		: m_TotalSize(TSize)
	{
		m_MemoryBlock = static_cast<TBlockType*>(malloc(TSize));
		m_CurrentPtr = m_MemoryBlock;
	}

	template<typename TBlockType, size_t TSize>
	LinearAllocator<TBlockType, TSize>::~LinearAllocator()
	{
		if (!m_MemoryBlock)
			return;
		free(m_MemoryBlock);
		m_MemoryBlock = nullptr;
		m_CurrentPtr = nullptr;
		m_TotalSize = 0;

#ifdef SUBSTRATE_DETAILS_ENABLED
		m_CurrentAllocationCount = 0;
		m_TotalAllocationCount = 0;
		m_ResetCount = 0;
#endif
	}

	template<typename TBlockType, size_t TSize>
	TBlockType* LinearAllocator<TBlockType, TSize>::Allocate(uint32_t count)
	{
		static constexpr size_t size = sizeof(TBlockType);
		size_t total = count * size;
		if (static_cast<size_t>(m_CurrentPtr - m_MemoryBlock) + total > m_TotalSize)
			return nullptr;

		TBlockType* allocatedMemory = m_CurrentPtr;
		m_CurrentPtr += total;

#ifdef SUBSTRATE_DETAILS_ENABLED
		m_CurrentAllocationCount += count;
		m_TotalAllocationCount += count;
#endif

		return allocatedMemory;
	}

	template<typename TBlockType, size_t TSize>
	void LinearAllocator<TBlockType, TSize>::Reset()
	{
		m_CurrentPtr = m_MemoryBlock;

#ifdef SUBSTRATE_DETAILS_ENABLED
		m_CurrentAllocationCount = 0;
		m_ResetCount++;
#endif
	}

}