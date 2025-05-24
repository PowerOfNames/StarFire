#include "sfpch.h"
#include "StarFire/Core/Logging.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace StarFire {


	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_CoreDebugLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientDebugLogger;
	std::shared_ptr<spdlog::logger> Log::s_RendererLogger;
	std::shared_ptr<spdlog::logger> Log::s_RendererDebugLogger;

	void Log::Init()
	{
		// %^: start color range; %T:time; %e:ms; %n logger name; %v: message; %$: end color range
		s_CoreLogger = spdlog::stdout_color_mt("STARFIRE");
		s_CoreLogger->set_pattern("%^[%T%e] [%n] %v%$");
		s_CoreLogger->set_level(spdlog::level::level_enum::trace);
		
		s_CoreDebugLogger = spdlog::stdout_color_mt("STARFIRE - DEBUG");
		s_CoreDebugLogger->set_pattern("%^[%T%e] %! [Line:%#] %v%$");
		s_CoreDebugLogger->set_level(spdlog::level::level_enum::debug);

		s_ClientLogger = spdlog::stdout_color_mt("CLIENT");
		s_ClientLogger->set_pattern("%^[%T%e] [%n] %v%$");
		s_ClientLogger->set_level(spdlog::level::level_enum::trace);

		s_ClientDebugLogger = spdlog::stdout_color_mt("APP - DEBUG");
		s_ClientDebugLogger->set_pattern("%^[%T%e] %! [Line:%#] %v%$");
		s_ClientDebugLogger->set_level(spdlog::level::level_enum::debug);

		s_RendererLogger = spdlog::stdout_color_mt("AURORA");
		s_RendererLogger->set_pattern("%^[%T%e] [%n] %v%$");
		s_RendererLogger->set_level(spdlog::level::level_enum::trace);

		s_RendererDebugLogger = spdlog::stdout_color_mt("AURORA - DEBUG");
		s_RendererDebugLogger->set_pattern("%^[%T%e] %! [Line:%#] %v%$");
		s_RendererDebugLogger->set_level(spdlog::level::level_enum::debug);

		SF_CORE_INFO("Initialized spdlog (Version {}.{}.{})", SPDLOG_VER_MAJOR, SPDLOG_VER_MINOR, SPDLOG_VER_PATCH);
	}

	void Log::Shutdown()
	{
		spdlog::shutdown();		
	}
}
