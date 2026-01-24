#pragma once

#include "Substrate/BaseHandle.h"
#include "Substrate/Defines.h"




namespace Substrate {


	template<typename THandleType, typename TBlockType>
		requires HandleTypeCheck<THandleType>
	class PoolAllocator
	{
	public:
		static constexpr THandleType INVALID_HANDLE = BaseHandle::INVALID_HANDLE;

		PoolAllocator(size_t size);
		~PoolAllocator() = default;

		void Destroy();

		THandleType Allocate(size_t size, size_t alignment = 1);
		TBlockType* GetPointerFromHandle(THandleType handle);
		void Free(THandleType handle);

#ifdef SUBSTRATE_DETAILS_ENABLED
		inline virtual size_t GetUsedMemory() const override { return 0; }
		inline virtual size_t GetTotalMemory() const override { return 0; }
		inline virtual size_t GetAllocationCount() const override { return 0; }
#endif
	private:
		using InternalHandle = DefineHandle<1, (sizeof(THandleType) * 8) - 1, THandleType>;

		size_t m_TotalSize;
	};
}
