#include "sfpch.h"
#include "StarFire/Core/Core.h"
#include "StarFire/Core/Window.h"

#ifdef SF_PLATFORM_WINDOWS
	#include "Platform/Windows/WindowsWindow.h"
#endif

namespace StarFire {

	std::unique_ptr<Window> Window::Create(const WindowSpecification& specs /* = WindowSpecification() */)
	{
#ifdef SF_PLATFORM_WINDOWS
		SF_CORE_INFO("Selecting WindowWindow");
		return std::make_unique<Platform::WindowsWindow>(specs);
#else
		SF_CORE_INFO("No window created");
		return nullptr;
#endif
	}


}
