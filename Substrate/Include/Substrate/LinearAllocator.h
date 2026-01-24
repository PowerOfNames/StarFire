#pragma once
#include "Substrate/Defines.h"
#include "Substrate/Exceptions.h"
#include "Substrate/UtilityFunctions.h"

#include <cstdlib>

namespace Substrate {


	template<typename TBlockType>
	class LinearAllocator
	{
	public:
		LinearAllocator(size_t size);
		~LinearAllocator();

		TBlockType* Allocate(uint32_t count = 1);		
		struct ArrayView
		{
			TBlockType* Data;
			size_t Count;

			TBlockType& operator[](size_t index)
			{
				if (index > Count)
					throw ArrayIndexOutOfBoundsException("Index" SST_STRINGIFY(index) "out of scope (" SST_STRINGIFY(Count) ")");
				return Data[index];
			}
			const TBlockType& operator[](size_t index) const
			{
				if (index > Count)
					throw ArrayIndexOutOfBoundsException("Index" SST_STRINGIFY(index) "out of scope (" SST_STRINGIFY(Count) ")");
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
		inline size_t GetUsedMemory() const { return static_cast<size_t>(m_CurrentPtr - m_MemoryBlock); }
		inline size_t GetTotalMemory() const { return m_TotalSize; }
		inline size_t GetAllocationCount() const 
		{
			static constexpr size_t size = sizeof(TBlockType);
			return GetUsedMemory() / size;
		}
#endif
	private:
		size_t m_TotalSize = 0;
		TBlockType* m_MemoryBlock = nullptr;
		TBlockType* m_CurrentPtr = nullptr;
	};

	template<typename TBlockType>
	LinearAllocator<TBlockType>::LinearAllocator(size_t size)
		: m_TotalSize(size)
	{
		m_MemoryBlock = static_cast<TBlockType*>(malloc(size));
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
		return allocatedMemory;
	}

	template<typename TBlockType>
	void LinearAllocator<TBlockType>::Reset()
	{
		m_CurrentPtr = m_MemoryBlock;
	}

}