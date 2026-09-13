#pragma once

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

namespace StarFire {

	class Log
	{
	public:
		static void Init();
		static void Shutdown();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

		inline static std::shared_ptr<spdlog::logger>& GetRendererLogger() { return s_RendererLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
		static std::shared_ptr<spdlog::logger> s_RendererLogger;
	};

}
	
// LEGEND
// TRACE	: (White)	Used for value tracing 		
// DEBUG	: (Cyan)	Used for debugging. Should be removed after debug happend.
// INFO		: (Green)	Used for Information (System, States...)
// WARN		: (Yellow)	Used for highlighting of potentially problematic behavior
// ERROR	: (Red)		Used for broken code paths / results
// CRITICAL	: (Marked)	Used for big NONOs -> should never be hits

#if defined(STARFIRE_DEBUG_MODE)
#define STARFIRE_TRACE(...)		SPDLOG_LOGGER_TRACE(StarFire::Log::GetCoreLogger(), __VA_ARGS__)
#define STARFIRE_DEBUG(...)		SPDLOG_LOGGER_DEBUG(StarFire::Log::GetCoreLogger(), __VA_ARGS__)
#define STARFIRE_INFO(...)		SPDLOG_LOGGER_INFO(StarFire::Log::GetCoreLogger(), __VA_ARGS__)
#define STARFIRE_WARN(...)		SPDLOG_LOGGER_WARN(StarFire::Log::GetCoreLogger(), __VA_ARGS__)
#define STARFIRE_ERROR(...)		SPDLOG_LOGGER_ERROR(StarFire::Log::GetCoreLogger(), __VA_ARGS__)
#define STARFIRE_CRITICAL(...)	SPDLOG_LOGGER_CRITICAL(StarFire::Log::GetCoreLogger(), __VA_ARGS__)

#define APP_TRACE(...)			SPDLOG_LOGGER_TRACE(StarFire::Log::GetClientLogger(), __VA_ARGS__)
#define APP_DEBUG(...)			SPDLOG_LOGGER_DEBUG(StarFire::Log::GetClientLogger(), __VA_ARGS__)
#define APP_INFO(...)			SPDLOG_LOGGER_INFO(StarFire::Log::GetClientLogger(), __VA_ARGS__)
#define APP_WARN(...)			SPDLOG_LOGGER_WARN(StarFire::Log::GetClientLogger(), __VA_ARGS__)
#define APP_ERROR(...)			SPDLOG_LOGGER_ERROR(StarFire::Log::GetClientLogger(), __VA_ARGS__)
#define APP_CRITICAL(...)		SPDLOG_LOGGER_CRITICAL(StarFire::Log::GetClientLogger(), __VA_ARGS__)

#define RENDERER_TRACE_HOOK(file, func, line, ...)		StarFire::Log::GetRendererLogger()->log(spdlog::source_loc{file, line, func}, spdlog::level::trace, __VA_ARGS__)
#define RENDERER_DEBUG_HOOK(file, func, line, ...)		StarFire::Log::GetRendererLogger()->log(spdlog::source_loc{file, line, func}, spdlog::level::debug, __VA_ARGS__)
#define RENDERER_INFO_HOOK(file, func, line, ...)		StarFire::Log::GetRendererLogger()->log(spdlog::source_loc{file, line, func}, spdlog::level::info, __VA_ARGS__)
#define RENDERER_WARN_HOOK(file, func, line, ...)		StarFire::Log::GetRendererLogger()->log(spdlog::source_loc{file, line, func}, spdlog::level::warn, __VA_ARGS__)
#define RENDERER_ERROR_HOOK(file, func, line, ...)		StarFire::Log::GetRendererLogger()->log(spdlog::source_loc{file, line, func}, spdlog::level::err, __VA_ARGS__)
#define RENDERER_CRITICAL_HOOK(file, func, line, ...)	StarFire::Log::GetRendererLogger()->log(spdlog::source_loc{file, line, func}, spdlog::level::critical, __VA_ARGS__)
//TODO: for release builds, log to files and error and critical to console
#elif defined(STARFIRE_RELEASE_MODE) || defined(STARFIRE_PROFILING_MODE)
#define STARFIRE_TRACE(...)		do {} while(false)
#define STARFIRE_DEBUG(...)		do {} while(false)
#define STARFIRE_INFO(...)		do {} while(false)
#define STARFIRE_WARN(...)		do {} while(false)
#define STARFIRE_ERROR(...)		SPDLOG_LOGGER_ERROR(StarFire::Log::GetCoreLogger(), __VA_ARGS__)
#define STARFIRE_CRITICAL(...)	SPDLOG_LOGGER_CRITICAL(StarFire::Log::GetCoreLogger(), __VA_ARGS__)

#define APP_TRACE(...)			do {} while(false)
#define APP_DEBUG(...)			do {} while(false)
#define APP_INFO(...)			do {} while(false)
#define APP_WARN(...)			do {} while(false)
#define APP_ERROR(...)			SPDLOG_LOGGER_ERROR(StarFire::Log::GetClientLogger(), __VA_ARGS__)
#define APP_CRITICAL(...)		SPDLOG_LOGGER_CRITICAL(StarFire::Log::GetClientLogger(), __VA_ARGS__)
	
#define RENDERER_DEBUG_HOOK(file, func, line, ...)		do {} while(false)
#define RENDERER_TRACE_HOOK(file, func, line, ...)		do {} while(false)
#define RENDERER_INFO_HOOK(file, func, line, ...)		do {} while(false)
#define RENDERER_WARN_HOOK(file, func, line, ...)		do {} while(false)
#define RENDERER_ERROR_HOOK(file, func, line, ...)		StarFire::Log::GetRendererLogger()->log(spdlog::source_loc{file, line, func}, spdlog::level::err, __VA_ARGS__)
#define RENDERER_CRITICAL_HOOK(file, func, line, ...)	StarFire::Log::GetRendererLogger()->log(spdlog::source_loc{file, line, func}, spdlog::level::critical, __VA_ARGS__)
//TODO: for distribution build, only log to files. No console sink enabled
#elif defined(STARFIRE_DIST_MODE)
#define STARFIRE_TRACE(...)		do {} while(false)
#define STARFIRE_DEBUG(...)		do {} while(false)
#define STARFIRE_INFO(...)		do {} while(false)
#define STARFIRE_WARN(...)		do {} while(false)
#define STARFIRE_ERROR(...)		do {} while(false)
#define STARFIRE_CRITICAL(...)	do {} while(false)

#define APP_TRACE(...)			do {} while(false)
#define APP_DEBUG(...)			do {} while(false)
#define APP_INFO(...)			do {} while(false)
#define APP_WARN(...)			do {} while(false)
#define APP_ERROR(...)			do {} while(false)
#define APP_CRITICAL(...)		do {} while(false)

#define RENDERER_TRACE_HOOK(...)	do {} while(false)
#define RENDERER_DEBUG_HOOK(...)	do {} while(false)
#define RENDERER_INFO_HOOK(...)		do {} while(false)
#define RENDERER_WARN_HOOK(...)		do {} while(false)
#define RENDERER_ERROR_HOOK(...)	do {} while(false)
#define RENDERER_CRITICAL_HOOK(...)	do {} while(false)
#endif






