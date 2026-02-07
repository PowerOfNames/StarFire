#pragma once
#if defined(STARFIRE_DEBUG_MODE)
#include <iostream>
#endif
#include "StarFire/Core/Application.h"
#include "StarFire/Utility/Timer.h"
#include "StarFire/Core/Logging.h"

#if defined(STARFIRE_PLATFORM_WINDOWS)
extern StarFire::Application* StarFire::CreateApplication(int argc, char** argv);

int main(int argc, char** argv)
{
	StarFire::Log::Init();
	StarFire::Utils::Timer timer;

	//SF_PROFILE_BEGIN_SESSION("Startup", "profiling/StarFireProfile-Startup.json");
	auto app = StarFire::CreateApplication(argc, argv);
	//SF_PROFILE_END_SESSION();

	SF_CORE_TRACE("App creation took {}ms", timer.TimestampMilli());

	//SF_PROFILE_BEGIN_SESSION("Running", "profiling/StarFireProfile-Running.json");
	app->Run();
	//SF_PROFILE_END_SESSION();

	SF_CORE_TRACE("Application ran {}s", timer.Timestamp());

	//SF_PROFILE_BEGIN_SESSION("Shutdown", "profiling/StarFireProfile-Shutdown.json");
	delete app;
	//SF_PROFILE_END_SESSION();

	SF_CORE_TRACE("Application shutdown took {}ms", timer.TimestampMilli());
	SF_CORE_TRACE("Closing application after {}s", timer.ElapsedTime());
	StarFire::Log::Shutdown();

#if defined(STARFIRE_DEBUG_MODE)
	std::cin.get();
#endif
}
#endif


