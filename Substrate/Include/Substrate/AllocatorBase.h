#pragma once
#include "Substrate/Defines.h"

namespace Substrate {

	class AllocatorBase
	{
	public:
		virtual ~AllocatorBase() = default;

#ifdef SUBSTRATE_DETAILS_ENABLED
		virtual size_t GetTotalMemory() const = 0;
		virtual size_t GetMaxAllocationCount() const = 0;
		virtual size_t GetUsedMemory() const = 0;
		virtual size_t GetCurrentAllocationCount() const = 0;
		virtual size_t GetTotalAllocationCount() const = 0;
		virtual size_t GetResetCount() const = 0;
#endif
	};
}