#pragma once

#include "Aurora/Core/Logging.h"
#include "Substrate/RefPtr.h"

#ifdef AURORA_DEBUG_MODE
#define AURORA_ASSERT_ENABLED
#endif

#ifdef AURORA_ASSERT_ENABLED
#define AURORA_ASSERT(x, ...) {if(!(x)) { AURORA_CRITICAL("Assertion failes: {}", __VA_ARGS__); __debugbreak();} }
#else
#define AURORA_ASSERT(x, ...) do{}while(0)
#endif

namespace Aurora {

	template<typename TRefCounted>
	using Ref = Substrate::RefPtr<TRefCounted>;
	template<typename TRefCounted, typename ... Args>
	constexpr Ref<TRefCounted> CreateRef(Args&& ... args)
	{
		return Ref<TRefCounted>(new TRefCounted(std::forward<Args>(args)...));
	}
}
