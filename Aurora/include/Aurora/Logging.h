#pragma once

#include <format>
#include <functional>
#include <string>

namespace Aurora {

	enum class LogLevel
	{
		ALL_NONE = 0,
		ALL_TRACE,
		ALL_INFO,
		ALL_DEBUG,
		ALL_WARN,
		ALL_ERROR,
		ALL_CRITICAL
	};

	using LogCallbackFn = std::function<void(LogLevel, const std::string&)>;

	class Log
	{
	public:
		inline static bool HasCallbackSet() { return s_Callback != nullptr; }
		static void SetCallback(LogCallbackFn callback);
		static void Shutdown();

		template<typename... Args>
		static void CombineCallbackArgs(LogLevel severityLevel, std::format_string<Args...> format, Args&&... args)
		{			
			std::string formatted = std::format(format, std::forward<Args>(args)...);
			Log::Message(severityLevel, formatted);
		}

		static void Message(LogLevel level, const std::string& message);

	private:
		inline static LogCallbackFn s_Callback;
	};

}


// LEGEND
// TRACE	: (White)	Used for value tracing 		
// INFO		: (Green)	Used for Information (System, States...)
// WARN		: (Yellow)	Used for highlighting of potentially problematic behavior
// ERROR	: (Red)		Used for broken code paths / results
// CRITICAL	: (Marked)	Used for big NONOs -> should never be hit

// DEBUG	: (Cyan)	Used for debugging. Should be removed after debug happend.


#define AURORA_TRACE(...)		Aurora::Log::CombineCallbackArgs(Aurora::LogLevel::ALL_TRACE, __VA_ARGS__)
#define AURORA_INFO(...)		Aurora::Log::CombineCallbackArgs(Aurora::LogLevel::ALL_INFO, __VA_ARGS__)
#define AURORA_WARN(...)		Aurora::Log::CombineCallbackArgs(Aurora::LogLevel::ALL_WARN, __VA_ARGS__)
#define AURORA_ERROR(...)		Aurora::Log::CombineCallbackArgs(Aurora::LogLevel::ALL_ERROR, __VA_ARGS__)
#define AURORA_CRITICAL(...)	Aurora::Log::CombineCallbackArgs(Aurora::LogLevel::ALL_CRITICAL, __VA_ARGS__)
													   
#define AURORA_DEBUG_LOG(...)	Aurora::Log::CombineCallbackArgs(Aurora::LogLevel::ALL_DEBUG, __VA_ARGS__)

