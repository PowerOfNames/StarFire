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
		STARFIRE_TRACE("Selecting WindowsWindow");
		return CreateScope<Platform::WindowsWindow>(specs);
#else
		STARFIRE_VALIDATE(false, "Window::Create() is not implemented for this platform!");
		return nullptr;
#endif
	}


}
