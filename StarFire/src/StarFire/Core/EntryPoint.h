#pragma once

#ifdef SF_PLATFORM_WINDOWS
extern StarFire::Application* StarFire::CreateApplication(int argc, char** argv);

int main(int argc, char** argv)
{
	StarFire::Log::Init();

	//SF_PROFILE_BEGIN_SESSION("Startup", "profiling/StarFireProfile-Startup.json");
	auto app = StarFire::CreateApplication(argc, argv);
	//SF_PROFILE_END_SESSION();

	//SF_PROFILE_BEGIN_SESSION("Running", "profiling/StarFireProfile-Running.json");
	app->Run();
	//SF_PROFILE_END_SESSION();

	//SF_PROFILE_BEGIN_SESSION("Shutdown", "profiling/StarFireProfile-Shutdown.json");
	delete app;
	//SF_PROFILE_END_SESSION();

	StarFire::Log::Shutdown();
}
#endif


