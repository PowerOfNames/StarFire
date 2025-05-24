#include "sfpch.h"
#include "StarFire/Core/Logging.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace StarFire {


	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;
	std::shared_ptr<spdlog::logger> Log::s_RendererLogger;

	void Log::Init()
	{
		// %^: start color range; %T:time; %e:ms; %n logger name; %v: message; %$: end color range
		s_CoreLogger = spdlog::stdout_color_mt("STARFIRE");
		s_CoreLogger->set_pattern("%^[%T%e] [%n] %! [Line:%#] %v%$");
		s_CoreLogger->set_level(spdlog::level::level_enum::trace);

		s_ClientLogger = spdlog::stdout_color_mt("CLIENT");
		s_ClientLogger->set_pattern("%^[%T%e] [%n] %! [Line:%#] %v%$");
		s_ClientLogger->set_level(spdlog::level::level_enum::trace);

		s_RendererLogger = spdlog::stdout_color_mt("AURORA");
		s_RendererLogger->set_pattern("%^[%T%e] [%n] %! [Line:%#] %v%$");
		s_RendererLogger->set_level(spdlog::level::level_enum::trace);

		SF_CORE_INFO("Initialized spdlog (Version {}.{}.{})", SPDLOG_VER_MAJOR, SPDLOG_VER_MINOR, SPDLOG_VER_PATCH);
	}

	void Log::Shutdown()
	{
		spdlog::shutdown();		
	}
}
