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
	int exitCode = EXIT_FAILURE;
	StarFire::Utils::Timer timer;

	{
		Scope<StarFire::Application> app;
		try
		{
			app.reset(StarFire::CreateApplication(argc, argv));

			if (app->Init())
			{
				STARFIRE_TRACE("App creation took {}ms", timer.TimestampMilli());
				app->Run();
				exitCode = EXIT_SUCCESS;
				STARFIRE_TRACE("Application ran {}s", timer.Timestamp());
			}
			else
				STARFIRE_CRITICAL("Failed to initialize application");

		}
		catch (const std::exception& e)
		{
			STARFIRE_CRITICAL("Unhandled exception: {}", e.what());
			exitCode = EXIT_FAILURE;
		}
		catch (...)
		{
			STARFIRE_CRITICAL("Unknown exception.");
			exitCode = EXIT_FAILURE;
		}
		STARFIRE_TRACE("Application shutdown took {}ms", timer.TimestampMilli());
		STARFIRE_TRACE("Closing application after {}s", timer.ElapsedTime());
	}
	StarFire::Log::Shutdown();
	return exitCode;
}
#endif


