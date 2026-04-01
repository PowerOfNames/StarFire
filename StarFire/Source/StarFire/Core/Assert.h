#pragma once

#ifdef STARFIRE_DEBUG_MODE
#define SF_ENABLE_ASSERT
#endif

#ifdef SF_ENABLE_ASSERT
#define SF_ASSERT(x, ...) { if(!(x)) { SF_ERROR("Assertion fails: {0}", __VA_ARGS__); __debugbreak(); } }
#define SF_CORE_ASSERT(x, ...) { if(!(x)) { SF_CORE_ERROR("Assertion fails: {0}", __VA_ARGS__); __debugbreak(); } }
#else
#define SF_ASSERT(x, ...)
#define SF_CORE_ASSERT(x, ...)
#endif