#pragma once
#include "Substrate/Defines.h"
#include "Substrate/UtilityFunctions.h"

#include <cstdlib>

namespace Substrate {

	
	class StackAllocator
	{
	public:
		StackAllocator(size_t size);
		~StackAllocator();

		template<typename TBlockType>
		TBlockType* Allocate();
		void Pop();
		void Reset();

#ifdef SUBSTRATE_DETAILS_ENABLED
		inline size_t GetUsedMemory() const { return static_cast<size_t>(m_CurrentOffset); }
		inline size_t GetTotalMemory() const { return m_TotalSize; }
		inline size_t GetAllocationCount() const { return m_AllocationCount; }
#endif
	private:

		void* m_MemoryBlock = nullptr;
		size_t m_CurrentOffset = 0;
		size_t m_TotalSize = 0;
		size_t m_AllocationCount = 0;
	};

	StackAllocator::StackAllocator(size_t size)
		: m_TotalSize(size)
	{
		m_MemoryBlock = malloc(size);		
	}

	StackAllocator::~StackAllocator()
	{
		if (!m_MemoryBlock)
			return;

		free(m_MemoryBlock);
	}

	/// <summary>
	/// Tries to allocate space for the requested type.
	/// Will return nullptr if there is not enough memory left in the stack.
	/// </summary>
	/// <typeparam name="TBlockType"></typeparam>
	/// <returns></returns>
	template<typename TBlockType>
	TBlockType* StackAllocator::Allocate()
	{
		//Check if the requested allocation fits in the remaining memory
		// We realign the blocksize immediately to a uint64_t boundary, because we will attach a uint64_t header after the block
		const size_t blockSize = AlignUpToMultipleOfMinAlignment(sizeof(TBlockType), sizeof(uint64_t));
		if ((m_CurrentOffset + blockSize + sizeof(uint64_t)) > m_TotalSize)
			return nullptr;

		//Allocate the block (We dont need to handle alignment here, because we will attach a uint64_t header after the block)
		TBlockType* blockPtr = reinterpret_cast<TBlockType*>(reinterpret_cast<uint8_t*>(m_MemoryBlock) + m_CurrentOffset);
		m_CurrentOffset += blockSize;
		m_AllocationCount++;
		
		//Store the block size in the header.
		uint64_t* header = reinterpret_cast<uint64_t*>(reinterpret_cast<uint8_t*>(m_MemoryBlock) + m_CurrentOffset);
		*header = static_cast<uint64_t>(blockSize);
		m_CurrentOffset += sizeof(uint64_t);

		return blockPtr;
	}

	void StackAllocator::Pop()
	{
		if (m_CurrentOffset == 0)
			return;
		//Get the size of the last allocated block
		uint64_t* header = reinterpret_cast<uint64_t*>(reinterpret_cast<uint8_t*>(m_MemoryBlock) + m_CurrentOffset - sizeof(uint64_t));
		size_t blockSize = static_cast<size_t>(*header);
		//Move the current offset back to free the last allocated block
		m_CurrentOffset -= (blockSize + sizeof(uint64_t));
		m_AllocationCount--;
	}

	void StackAllocator::Reset()
	{
		m_CurrentOffset = 0;
		m_AllocationCount = 0;
	}
}
