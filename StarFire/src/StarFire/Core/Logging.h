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
// CRITICAL	: (Marked)	Used for big NONOs -> should never be hit

#if defined(STARFIRE_DEBUG_MODE)
#define SF_CORE_TRACE(...)		SPDLOG_LOGGER_TRACE(StarFire::Log::GetCoreLogger(), __VA_ARGS__)
#define SF_CORE_DEBUG(...)		SPDLOG_LOGGER_DEBUG(StarFire::Log::GetCoreLogger(), __VA_ARGS__)
#define SF_CORE_INFO(...)		SPDLOG_LOGGER_INFO(StarFire::Log::GetCoreLogger(), __VA_ARGS__)
#define SF_CORE_WARN(...)		SPDLOG_LOGGER_WARN(StarFire::Log::GetCoreLogger(), __VA_ARGS__)
#define SF_CORE_ERROR(...)		SPDLOG_LOGGER_ERROR(StarFire::Log::GetCoreLogger(), __VA_ARGS__)
#define SF_CORE_CRITICAL(...)	SPDLOG_LOGGER_CRITICAL(StarFire::Log::GetCoreLogger(), __VA_ARGS__)

#define SF_TRACE(...)			SPDLOG_LOGGER_TRACE(StarFire::Log::GetClientLogger(), __VA_ARGS__)
#define SF_DEBUG(...)			SPDLOG_LOGGER_DEBUG(StarFire::Log::GetClientLogger(), __VA_ARGS__)
#define SF_INFO(...)			SPDLOG_LOGGER_INFO(StarFire::Log::GetClientLogger(), __VA_ARGS__)
#define SF_WARN(...)			SPDLOG_LOGGER_WARN(StarFire::Log::GetClientLogger(), __VA_ARGS__)
#define SF_ERROR(...)			SPDLOG_LOGGER_ERROR(StarFire::Log::GetClientLogger(), __VA_ARGS__)
#define SF_CRITICAL(...)		SPDLOG_LOGGER_CRITICAL(StarFire::Log::GetClientLogger(), __VA_ARGS__)

#define SF_R_CORE_TRACE(file, func, line, ...)		StarFire::Log::GetRendererLogger()->log(spdlog::source_loc{file, line, func}, spdlog::level::trace, __VA_ARGS__)
#define SF_R_CORE_DEBUG(file, func, line, ...)		StarFire::Log::GetRendererLogger()->log(spdlog::source_loc{file, line, func}, spdlog::level::debug, __VA_ARGS__)
#define SF_R_CORE_INFO(file, func, line, ...)		StarFire::Log::GetRendererLogger()->log(spdlog::source_loc{file, line, func}, spdlog::level::info, __VA_ARGS__)
#define SF_R_CORE_WARN(file, func, line, ...)		StarFire::Log::GetRendererLogger()->log(spdlog::source_loc{file, line, func}, spdlog::level::warn, __VA_ARGS__)
#define SF_R_CORE_ERROR(file, func, line, ...)		StarFire::Log::GetRendererLogger()->log(spdlog::source_loc{file, line, func}, spdlog::level::err, __VA_ARGS__)
#define SF_R_CORE_CRITICAL(file, func, line, ...)	StarFire::Log::GetRendererLogger()->log(spdlog::source_loc{file, line, func}, spdlog::level::critical, __VA_ARGS__)
//TODO: for release builds, log to files and error and critical to console
#elif defined(STARFIRE_RELEASE_MODE)
#define SF_CORE_TRACE(...)	
#define SF_CORE_DEBUG(...)	
#define SF_CORE_INFO(...)	
#define SF_CORE_WARN(...)	
#define SF_CORE_ERROR(...)		SPDLOG_LOGGER_ERROR(StarFire::Log::GetCoreLogger(), __VA_ARGS__)
#define SF_CORE_CRITICAL(...)	SPDLOG_LOGGER_CRITICAL(StarFire::Log::GetCoreLogger(), __VA_ARGS__)

#define SF_TRACE(...)	
#define SF_DEBUG(...)	
#define SF_INFO(...)	
#define SF_WARN(...)	
#define SF_ERROR(...)			SPDLOG_LOGGER_ERROR(StarFire::Log::GetClientLogger(), __VA_ARGS__)
#define SF_CRITICAL(...)		SPDLOG_LOGGER_CRITICAL(StarFire::Log::GetClientLogger(), __VA_ARGS__)

#define SF_R_CORE_TRACE(...)	
#define SF_R_CORE_DEBUG(...)	
#define SF_R_CORE_INFO(...)		
#define SF_R_CORE_WARN(...)		
#define SF_R_CORE_ERROR(...)	SPDLOG_LOGGER_ERROR(StarFire::Log::GetRendererLogger(), __VA_ARGS__)
#define SF_R_CORE_CRITICAL(...)	SPDLOG_LOGGER_CRITICAL(StarFire::Log::GetRendererLogger(), __VA_ARGS__)
//TODO: for distribution build, only log to files. No console sink enabled
#elif defined(STARFIRE_DIST_MODE)
#define SF_CORE_TRACE(...)	
#define SF_CORE_DEBUG(...)	
#define SF_CORE_INFO(...)	
#define SF_CORE_WARN(...)	
#define SF_CORE_ERROR(...)		
#define SF_CORE_CRITICAL(...)	

#define SF_TRACE(...)	
#define SF_DEBUG(...)	
#define SF_INFO(...)	
#define SF_WARN(...)	
#define SF_ERROR(...)			
#define SF_CRITICAL(...)		

#define SF_R_CORE_TRACE(...)	
#define SF_R_CORE_DEBUG(...)	
#define SF_R_CORE_INFO(...)		
#define SF_R_CORE_WARN(...)		
#define SF_R_CORE_ERROR(...)	
#define SF_R_CORE_CRITICAL(...)	
#endif






