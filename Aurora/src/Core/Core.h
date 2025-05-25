#pragma once

#include "Core/Logging.h"

#ifdef AURORA_DEBUG_MODE
#define AURORA_ASSERT_ENABLED
#endif

#ifdef AURORA_ASSERT_ENABLED
#define AURORA_ASSERT(x, ...) {if(!x) { AURORA_CRITICAL("Assertion failes: {0}", __VA_ARGS__); __debugbreak();} }
#else
#define AURORA_ASSERT(x, ...) 
#endif

#include <memory>

namespace Aurora {

	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
}
