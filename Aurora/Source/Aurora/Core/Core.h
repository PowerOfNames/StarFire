#pragma once

#include "Aurora/Core/Logging.h"
#include "Substrate/RefPtr.h"
#include "Aurora/Core/Defines.h"
#include "Aurora/Profiling/Profiling.h"

#ifdef AURORA_DEBUG_MODE
#define AURORA_ASSERT_ENABLED
#endif

#ifdef AURORA_ASSERT_ENABLED
#define AURORA_ASSERT(x, ...) {if(!(x)) { AURORA_CRITICAL("Assertion failes: {}", __VA_ARGS__); __debugbreak();} }
#else
#define AURORA_ASSERT(x, ...) do{}while(0)
#endif
