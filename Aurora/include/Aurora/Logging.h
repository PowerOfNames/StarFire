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




#define AURA_TRACE(...)		Aurora::Log::CombineCallbackArgs(Aurora::LogLevel::ALL_TRACE, __VA_ARGS__)
#define AURA_DEBUG(...)		Aurora::Log::CombineCallbackArgs(Aurora::LogLevel::ALL_DEBUG, __VA_ARGS__)
#define AURA_INFO(...)		Aurora::Log::CombineCallbackArgs(Aurora::LogLevel::ALL_INFO, __VA_ARGS__)
#define AURA_WARN(...)		Aurora::Log::CombineCallbackArgs(Aurora::LogLevel::ALL_WARN, __VA_ARGS__)
#define AURA_ERROR(...)		Aurora::Log::CombineCallbackArgs(Aurora::LogLevel::ALL_ERROR, __VA_ARGS__)
#define AURA_CRITICAL(...)	Aurora::Log::CombineCallbackArgs(Aurora::LogLevel::ALL_CRITICAL, __VA_ARGS__)
							   

