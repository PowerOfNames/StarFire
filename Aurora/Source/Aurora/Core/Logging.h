#pragma once
#include "Aurora/Logging/LogLevel.h"

#include <format>
#include <string>
#include <string_view>

namespace Aurora {	

	using LogCallbackFn = void(*)(LogLevel, const std::string&, const char* file, const char* func, int line);

	class Log
	{
	public:
		inline static bool HasCallbackSet() { return s_Callback != nullptr; }
		static void SetCallback(LogCallbackFn callback);
		static void Shutdown();

		template<typename... Args>
		static void CombineCallbackArgs(LogLevel severityLevel, const char* file, const char* func, int line, std::string_view format, Args&&... args)
		{
			try
			{
				std::string formatted = std::vformat(format, std::make_format_args(args...));
				Log::Message(severityLevel, formatted, file, func, line);
			}
			catch (const std::format_error& e)
			{				
				Log::Message(severityLevel, std::string("Format error: ") + e.what(), file, func, line);
			}
		}

	private:
		static void Message(LogLevel level, const std::string& message, const char* file, const char* func, int line);
		inline static LogCallbackFn s_Callback;
	};

}


// LEGEND
// TRACE	: (White)	Used for value tracing 		
// DEBUG	: (Cyan)	Used for debugging. Should be removed after debug happend.
// INFO		: (Green)	Used for Information (System, States...)
// WARN		: (Yellow)	Used for highlighting of potentially problematic behavior
// ERROR	: (Red)		Used for broken code paths / results
// CRITICAL	: (Marked)	Used for big NONOs -> should never be hit

// Trace is the one level that is switchable inside a Debug build - it is the high-volume one.
// The default lives here, next to the only code that reads it: Logging.h is reached through Core.h /
// VulkanDebug.h by translation units that never include Aurora/Aurora.h, so a definition in a header
// they do not see would evaluate to 0 and silently disable trace in most of the engine.
// Override by defining ENABLE_TRACE before this header, or as a compiler define.
#ifndef ENABLE_TRACE
#define ENABLE_TRACE 1
#endif

#if defined(AURORA_DEBUG_MODE)
#if ENABLE_TRACE == 1
#define AURORA_TRACE(fmt, ...)		Aurora::Log::CombineCallbackArgs(Aurora::LogLevel::LOG_LEVEL_TRACE, __FILE__, __func__, __LINE__, fmt, __VA_ARGS__)
#else
#define AURORA_TRACE(fmt, ...)		do{} while(false)
#endif
#define AURORA_DEBUG(fmt, ...)		Aurora::Log::CombineCallbackArgs(Aurora::LogLevel::LOG_LEVEL_DEBUG, __FILE__, __func__, __LINE__, fmt, __VA_ARGS__)
#define AURORA_INFO(fmt, ...)		Aurora::Log::CombineCallbackArgs(Aurora::LogLevel::LOG_LEVEL_INFO, __FILE__, __func__, __LINE__, fmt, __VA_ARGS__)
#define AURORA_WARN(fmt, ...)		Aurora::Log::CombineCallbackArgs(Aurora::LogLevel::LOG_LEVEL_WARN, __FILE__, __func__, __LINE__, fmt, __VA_ARGS__)
#define AURORA_ERROR(fmt, ...)		Aurora::Log::CombineCallbackArgs(Aurora::LogLevel::LOG_LEVEL_ERROR, __FILE__, __func__, __LINE__, fmt, __VA_ARGS__)
#define AURORA_CRITICAL(fmt, ...)	Aurora::Log::CombineCallbackArgs(Aurora::LogLevel::LOG_LEVEL_CRITICAL, __FILE__, __func__, __LINE__, fmt, __VA_ARGS__)
#elif defined(AURORA_RELEASE_MODE) || defined(AURORA_PROFILING_MODE)
#define AURORA_TRACE(fmt, ...)		do{} while(false)
#define AURORA_DEBUG(fmt, ...)		do{} while(false)
#define AURORA_INFO(fmt, ...)		do{} while(false)
#define AURORA_WARN(fmt, ...)		do{} while(false)
#define AURORA_ERROR(fmt, ...)		Aurora::Log::CombineCallbackArgs(Aurora::LogLevel::LOG_LEVEL_ERROR, __FILE__, __func__, __LINE__, fmt, __VA_ARGS__)
#define AURORA_CRITICAL(fmt, ...)	Aurora::Log::CombineCallbackArgs(Aurora::LogLevel::LOG_LEVEL_CRITICAL, __FILE__, __func__, __LINE__, fmt, __VA_ARGS__)
#endif

