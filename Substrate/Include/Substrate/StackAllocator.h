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
		inline size_t GetMaxedGenerationCount()		const override { return 0; }
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
		static_assert(alignof(TBlockType) <= alignof(uint64_t),
		"StackAllocator only supports types aligned to 8 bytes; over-aligned types need the alignment overhaul.");

		// header must stay 8-aligned, so never align to less than 8
		constexpr size_t alignment = std::max(alignof(TBlockType), alignof(uint64_t));
		constexpr size_t blockSize = Utility::AlignUpToMultipleOfMinAlignment(sizeof(TBlockType), sizeof(uint64_t));

		const size_t prevOffset = m_CurrentOffset;
		const size_t blockStart = Utility::AlignUpToMultipleOfMinAlignment(m_CurrentOffset, alignment); // ← the new line
		const size_t headerStart = blockStart + blockSize;

		if (headerStart + sizeof(uint64_t) > m_TotalSize)
			return nullptr;

		auto* base = static_cast<uint8_t*>(m_MemoryBlock);
		auto* block = reinterpret_cast<TBlockType*>(base + blockStart);
		auto* header = reinterpret_cast<uint64_t*>(base + headerStart);

		*header = static_cast<uint64_t>(prevOffset);   // store where to rewind — not blockSize
		m_CurrentOffset = headerStart + sizeof(uint64_t);

#ifdef SUBSTRATE_DETAILS_ENABLED
		m_CurrentAllocationCount++;
		m_TotalAllocationCount++;
#endif
		return block;
	}

	void StackAllocator::Pop()
	{
		if (m_CurrentOffset == 0)
			return;

		//Get the size of the last allocated block
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		uint64_t* header = reinterpret_cast<uint64_t*>(static_cast<uint8_t*>(m_MemoryBlock) + m_CurrentOffset - sizeof(uint64_t));
		//Move the current offset back to free the last allocated block
		m_CurrentOffset = static_cast<size_t>(*header);

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
