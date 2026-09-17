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

	auto app = StarFire::CreateApplication(argc, argv);

	STARFIRE_TRACE("App creation took {}ms", timer.TimestampMilli());

	app->Run();

	STARFIRE_TRACE("Application ran {}s", timer.Timestamp());

	delete app;

	STARFIRE_TRACE("Application shutdown took {}ms", timer.TimestampMilli());
	STARFIRE_TRACE("Closing application after {}s", timer.ElapsedTime());
	StarFire::Log::Shutdown();
}
#endif


