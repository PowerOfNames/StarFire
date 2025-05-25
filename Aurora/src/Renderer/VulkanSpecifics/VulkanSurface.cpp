#include "Renderer/VulkanSpecifics/VulkanSurface.h"

#include "Core/Core.h"

namespace Aurora { namespace VK {

	VulkanSurface::VulkanSurface(const SurfaceSpecification& specs)
		: m_Specification(specs)
	{
	}


	void VulkanSurface::Init()
	{
		AURORA_TRACE("Initializing VulkanSurface");
	}
	}
}