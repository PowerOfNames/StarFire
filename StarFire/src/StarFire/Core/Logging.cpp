#include "sfpch.h"
#include "StarFire/Core/Logging.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace StarFire {


	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;
	std::shared_ptr<spdlog::logger> Log::s_RendererLogger;

	void Log::Init()
	{
		//TODO: introduce file sink and use that for release and dist builds
#if defined(STARFIRE_DEBUG_MODE)
		spdlog::level::level_enum level = spdlog::level::trace;
#elif defined(STARFIRE_RELEASE_MODE)
		spdlog::level::level_enum level = spdlog::level::err;
#elif defined(STARFIRE_DIST_MODE)
		spdlog::level::level_enum level = spdlog::level::off;
#endif

		// %^: start color range; %T:time; %e:ms; %n logger name; %v: message; %$: end color range
		s_CoreLogger = spdlog::stdout_color_mt("STARFIRE");
		s_CoreLogger->set_pattern("%^[%T%e] [%n] %! [Line:%#] %v%$");
		s_CoreLogger->set_level(level);

		s_ClientLogger = spdlog::stdout_color_mt("CLIENT");
		s_ClientLogger->set_pattern("%^[%T%e] [%n] %! [Line:%#] %v%$");
		s_ClientLogger->set_level(level);

		s_RendererLogger = spdlog::stdout_color_mt("AURORA");
		s_RendererLogger->set_pattern("%^[%T%e] [%n] %! [Line:%#] %v%$");
		s_RendererLogger->set_level(level);

		SF_CORE_INFO("Initialized spdlog (Version {}.{}.{})", SPDLOG_VER_MAJOR, SPDLOG_VER_MINOR, SPDLOG_VER_PATCH);
	}

	void Log::Shutdown()
	{
		spdlog::shutdown();		
	}
}
