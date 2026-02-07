#include "sfpch.h"
#include "StarFire/Core/Core.h"

#include "StarFire/Core/Window.h"

#ifdef SF_PLATFORM_WINDOWS
	#include "Platform/Windows/WindowsWindow.h"
#endif

namespace StarFire {

	Scope<Window> Window::Create(const WindowSpecification& specs /* = WindowSpecification() */)
	{
#ifdef SF_PLATFORM_WINDOWS
		SF_CORE_TRACE("Selecting WindowsWindow");
		return CreateScope<Platform::WindowsWindow>(specs);
#else
		SF_CORE_WARN("No window created");
		return nullptr;
#endif
	}


}
