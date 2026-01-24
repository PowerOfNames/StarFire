#pragma once
#include "Substrate/Defines.h"
#include "Substrate/BaseHandle.h"

namespace Substrate {

	
	class StackAllocator
	{
	public:
		StackAllocator(size_t size);
		~StackAllocator() = default;
		
		void Destroy();

		void* Allocate(size_t size, size_t alignment = 1);
		void FreeLast();
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

}
