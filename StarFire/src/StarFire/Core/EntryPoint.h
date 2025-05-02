#pragma once
#ifdef SF_DEBUG
#include <iostream>
#endif
#include "StarFire/Utility/Timer.h"

#ifdef SF_PLATFORM_WINDOWS
extern StarFire::Application* StarFire::CreateApplication(int argc, char** argv);

int main(int argc, char** argv)
{
	StarFire::Log::Init();
	StarFire::Utils::Timer timer;

	//SF_PROFILE_BEGIN_SESSION("Startup", "profiling/StarFireProfile-Startup.json");
	auto app = StarFire::CreateApplication(argc, argv);
	//SF_PROFILE_END_SESSION();

	SF_CORE_INFO("App creation took {}ms", timer.TimestampMilli());

	//SF_PROFILE_BEGIN_SESSION("Running", "profiling/StarFireProfile-Running.json");
	app->Run();
	//SF_PROFILE_END_SESSION();

	SF_CORE_INFO("Application ran {}s", timer.Timestamp());

	//SF_PROFILE_BEGIN_SESSION("Shutdown", "profiling/StarFireProfile-Shutdown.json");
	delete app;
	//SF_PROFILE_END_SESSION();

	SF_CORE_INFO("Application shutdown took {}ms", timer.TimestampMilli());
	SF_CORE_INFO("Closing application after {}s", timer.ElapsedTime());
	StarFire::Log::Shutdown();

#ifdef SF_DEBUG
	std::cin.get();
#endif
}
#endif


