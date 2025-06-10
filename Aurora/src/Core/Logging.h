#pragma once
#include "Aurora/Logging/LogLevel.h"

#include <format>
#include <string>
#include <string_view>

namespace Aurora {	

	using LogCallbackFn = void(*)(LogLevel, const std::string&);

	class Log
	{
	public:
		inline static bool HasCallbackSet() { return s_Callback != nullptr; }
		static void SetCallback(LogCallbackFn callback);
		static void Shutdown();

		/*template<typename... Args>
		static void CombineCallbackArgs(LogLevel severityLevel, std::format_string<Args...> format, Args&&... args)
		{
			std::string formatted = std::format(format, std::forward<Args>(args)...);
			Log::Message(severityLevel, formatted);
		}*/

		template<typename... Args>
		static void CombineCallbackArgs(LogLevel severityLevel, std::string_view format, Args&&... args)
		{
			try
			{
				std::string formatted = std::vformat(format, std::make_format_args(args...));
				Log::Message(severityLevel, formatted);
			}
			catch (const std::format_error& e)
			{				
				Log::Message(severityLevel, std::string("Format error: ") + e.what());
			}
		}

	private:
		static void Message(LogLevel level, const std::string& message);
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

#define AURORA_TRACE(fmt, ...)		Aurora::Log::CombineCallbackArgs(Aurora::LogLevel::LOG_LEVEL_TRACE, fmt, __VA_ARGS__)
#define AURORA_DEBUG(fmt, ...)		Aurora::Log::CombineCallbackArgs(Aurora::LogLevel::LOG_LEVEL_DEBUG, fmt, __VA_ARGS__)
#define AURORA_INFO(fmt, ...)		Aurora::Log::CombineCallbackArgs(Aurora::LogLevel::LOG_LEVEL_INFO, fmt, __VA_ARGS__)
#define AURORA_WARN(fmt, ...)		Aurora::Log::CombineCallbackArgs(Aurora::LogLevel::LOG_LEVEL_WARN, fmt, __VA_ARGS__)
#define AURORA_ERROR(fmt, ...)		Aurora::Log::CombineCallbackArgs(Aurora::LogLevel::LOG_LEVEL_ERROR, fmt, __VA_ARGS__)
#define AURORA_CRITICAL(fmt, ...)	Aurora::Log::CombineCallbackArgs(Aurora::LogLevel::LOG_LEVEL_CRITICAL, fmt, __VA_ARGS__)
							   

