#pragma once
#include "Substrate/AllocatorBase.h"
#include "Substrate/Defines.h"
#include "Substrate/Exceptions.h"
#include "Substrate/UtilityFunctions.h"
#include "Substrate/ArrayView.h"

#include <cstdlib>
#include <sstream>

namespace Substrate {


	template<typename TBlockType>
	class LinearAllocator : public AllocatorBase
	{
	public:		
		LinearAllocator(size_t totalSize);
		~LinearAllocator();

		TBlockType* Allocate(uint32_t count = 1);

		ArrayView<TBlockType> AllocateArray(uint32_t count)
		{
			TBlockType* data = Allocate(count);
			if (!data)
				return { nullptr, 0 };
			return { data, count };
		}
		void Reset();

#ifdef SUBSTRATE_DETAILS_ENABLED
		inline size_t GetUsedMemory()				const override { return static_cast<size_t>(m_CurrentPtr - m_MemoryBlock); }
		inline size_t GetMaxAllocationCount()		const override { return static_cast<uint32_t>(m_TotalSize / sizeof(TBlockType)); }
		inline size_t GetTotalMemory()				const override { return m_TotalSize; }
		inline size_t GetTotalAllocationCount()		const override { return m_TotalAllocationCount; }
		inline size_t GetCurrentAllocationCount()	const override { return m_CurrentAllocationCount; }
		inline size_t GetMaxedGenerationCount()		const override { return 0; }
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

	template<typename TBlockType>
	LinearAllocator<TBlockType>::LinearAllocator(size_t totalSize)
		: m_TotalSize(totalSize)
	{
		m_MemoryBlock = static_cast<TBlockType*>(malloc(totalSize));
		m_CurrentPtr = m_MemoryBlock;
	}

	template<typename TBlockType>
	LinearAllocator<TBlockType>::~LinearAllocator()
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

	template<typename TBlockType>
	TBlockType* LinearAllocator<TBlockType>::Allocate(uint32_t count)
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

	template<typename TBlockType>
	void LinearAllocator<TBlockType>::Reset()
	{
		m_CurrentPtr = m_MemoryBlock;

#ifdef SUBSTRATE_DETAILS_ENABLED
		m_CurrentAllocationCount = 0;
		m_ResetCount++;
#endif
	}

}