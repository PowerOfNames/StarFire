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
		inline static std::shared_ptr<spdlog::logger>& GetCoreDebugLogger() { return s_CoreDebugLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientDebugLogger() { return s_ClientDebugLogger; }

		inline static std::shared_ptr<spdlog::logger>& GetRendererLogger() { return s_RendererLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetRendererDebugLogger() { return s_RendererDebugLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_CoreDebugLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
		static std::shared_ptr<spdlog::logger> s_ClientDebugLogger;
		static std::shared_ptr<spdlog::logger> s_RendererLogger;
		static std::shared_ptr<spdlog::logger> s_RendererDebugLogger;
	};

}
	
// LEGEND
// TRACE	: (White)	Used for value tracing 		
// INFO		: (Green)	Used for Information (System, States...)
// WARN		: (Yellow)	Used for highlighting of potentially problematic behavior
// ERROR	: (Red)		Used for broken code paths / results
// CRITICAL	: (Marked)	Used for big NONOs -> should never be hit

// DEBUG	: (Cyan)	Used for debugging. Should be removed after debug happend.


#define SF_CORE_TRACE(...)	SPDLOG_LOGGER_TRACE(StarFire::Log::GetCoreLogger(), __VA_ARGS__)
#define SF_CORE_INFO(...)	SPDLOG_LOGGER_INFO(StarFire::Log::GetCoreLogger(), __VA_ARGS__)
#define SF_CORE_WARN(...)	SPDLOG_LOGGER_WARN(StarFire::Log::GetCoreLogger(), __VA_ARGS__)
#define SF_CORE_ERROR(...)	SPDLOG_LOGGER_ERROR(StarFire::Log::GetCoreLogger(), __VA_ARGS__)
#define SF_CORE_CRITICAL(...)	SPDLOG_LOGGER_CRITICAL(StarFire::Log::GetCoreLogger(), __VA_ARGS__)

#define SF_CORE_DEBUG_LOG(...)	SPDLOG_LOGGER_DEBUG(StarFire::Log::GetCoreDebugLogger(), __VA_ARGS__)


#define SF_TRACE(...)	SPDLOG_LOGGER_TRACE(StarFire::Log::GetClientLogger(), __VA_ARGS__)
#define SF_INFO(...)	SPDLOG_LOGGER_INFO(StarFire::Log::GetClientLogger(), __VA_ARGS__)
#define SF_WARN(...)	SPDLOG_LOGGER_WARN(StarFire::Log::GetClientLogger(), __VA_ARGS__)
#define SF_ERROR(...)	SPDLOG_LOGGER_ERROR(StarFire::Log::GetClientLogger(), __VA_ARGS__)
#define SF_CRITICAL(...)	SPDLOG_LOGGER_CRITICAL(StarFire::Log::GetClientLogger(), __VA_ARGS__)

#define SF_DEBUG_LOG(...)	SPDLOG_LOGGER_DEBUG(StarFire::Log::GetClientDebugLogger(), __VA_ARGS__)


#define SF_R_CORE_TRACE(...)	SPDLOG_LOGGER_TRACE(StarFire::Log::GetRendererLogger(), __VA_ARGS__)
#define SF_R_CORE_INFO(...)	SPDLOG_LOGGER_INFO(StarFire::Log::GetRendererLogger(), __VA_ARGS__)
#define SF_R_CORE_WARN(...)	SPDLOG_LOGGER_WARN(StarFire::Log::GetRendererLogger(), __VA_ARGS__)
#define SF_R_CORE_ERROR(...)	SPDLOG_LOGGER_ERROR(StarFire::Log::GetRendererLogger(), __VA_ARGS__)
#define SF_R_CORE_CRITICAL(...)	SPDLOG_LOGGER_CRITICAL(StarFire::Log::GetRendererLogger(), __VA_ARGS__)
			
#define SF_R_CORE_DEBUG_LOG(...)	SPDLOG_LOGGER_DEBUG(StarFire::Log::GetRendererDebugLogger(), __VA_ARGS__)






