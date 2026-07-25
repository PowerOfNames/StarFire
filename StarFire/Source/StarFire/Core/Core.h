#pragma once

#include <memory>

#include "StarFire/Core/Logging.h"
#include "StarFire/Profiling/Profiling.h"
#include "Substrate/RefCounted.h"
#include "Substrate/RefPtr.h"

#ifdef _WIN32
#ifdef _WIN64
#define SF_PLATFORM_WINDOWS
#else 
#error "x86 builds are not supported!"
#endif
#elif defined(__APPLE__) || defined (__MAC__)
#include <TargetConditionals.h>
/*TARGET_OS_MAC exists on all the platforms.
 * so we must check all of them (in this order)
 * to ensure we are running on Mac and not some
 * other apple platform
 */
#if TARGET_IPHONE_SIMULATOR == 1
#error "IOS simulator not supported!"
#elif TARGET_OS_PHONE == 1
#define SF_PLATFORM_IOS
#error "IOS is not supported!"
#elif TARGET_OS_MAC
#define SF_PLATFORM_MACOS
#error "MACOS is not supported!"
#else
#error "Unknown apple platform!"
#endif
 /*We also have to check __ANDROID__ before __linux__
  * since android is based on the linux kernel,
  * it has __linux__ defined*/
#elif defined(__ANDROID__)
#define SF_PLATFORM_ANDROID
#error "Android is not supported!"
#elif defined (__linux__)
#define SF_PLATFORM_LINUX
#error "Linux is not supported!"
#else
// unknown compiler/platform
#error "Unknown platform!"
#endif


// DLL support
#ifdef SF_PLATFORM_WINDOWS
#if SF_DYNAMIC_LINK
#ifdef SF_BUILD_DLL
#define STARFIRE_API __declspec(dllexport)
#else
#define STARFIRE_API __declspec(dllimport)
#endif
#else
#define STARFIRE_API
#endif
#else
#error "StarFire only supports windows!"
#endif

#define SF_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) {return this->fn(std::forward<decltype(args)>(args)...); }