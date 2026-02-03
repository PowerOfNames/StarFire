#pragma once
#include "Substrate/AllocatorBase.h"
#include "Substrate/Defines.h"
#include "Substrate/UtilityFunctions.h"

#include <cstdlib>

namespace Substrate {

	
	class StackAllocator : public AllocatorBase
	{
	public:
		StackAllocator(size_t size);
		~StackAllocator();

		template<typename TBlockType>
		TBlockType* Allocate();
		void Pop();
		void Reset();

#ifdef SUBSTRATE_DETAILS_ENABLED
		inline size_t GetUsedMemory()				const override { return static_cast<size_t>(m_CurrentOffset); }
		inline size_t GetMaxAllocationCount()		const override { return m_TotalSize / 8; }
		inline size_t GetTotalMemory()				const override { return m_TotalSize; }
		inline size_t GetTotalAllocationCount()		const override { return m_TotalAllocationCount; }
		inline size_t GetCurrentAllocationCount()	const override { return m_CurrentAllocationCount; }
		inline size_t GetResetCount()				const override { return m_ResetCount; }
#endif
	private:

		void* m_MemoryBlock = nullptr;
		size_t m_CurrentOffset = 0;
		size_t m_TotalSize = 0;

#ifdef SUBSTRATE_DETAILS_ENABLED
		size_t m_CurrentAllocationCount = 0;
		size_t m_TotalAllocationCount = 0;
		size_t m_ResetCount = 0;
#endif
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
		m_MemoryBlock = nullptr;
		m_CurrentOffset = 0;
		m_TotalSize = 0;

#ifdef SUBSTRATE_DETAILS_ENABLED
		m_CurrentAllocationCount = 0;
		m_TotalAllocationCount = 0;
		m_ResetCount = 0;
#endif
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
		constexpr size_t blockSize = Utility::AlignUpToMultipleOfMinAlignment(sizeof(TBlockType), sizeof(uint64_t));
		if ((m_CurrentOffset + blockSize + sizeof(uint64_t)) > m_TotalSize)
			return nullptr;

		//Allocate the block (We dont need to handle alignment here, because we will attach a uint64_t header after the block)
		TBlockType* blockPtr = reinterpret_cast<TBlockType*>(reinterpret_cast<uint8_t*>(m_MemoryBlock) + m_CurrentOffset);
		m_CurrentOffset += blockSize;

#ifdef SUBSTRATE_DETAILS_ENABLED
		m_CurrentAllocationCount++;
		m_TotalAllocationCount++;
#endif
		
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

#ifdef SUBSTRATE_DETAILS_ENABLED
		m_CurrentAllocationCount--;
#endif
	}

	void StackAllocator::Reset()
	{
		m_CurrentOffset = 0;

#ifdef SUBSTRATE_DETAILS_ENABLED
		m_CurrentAllocationCount = 0;
		m_ResetCount++;
#endif
	}
}
