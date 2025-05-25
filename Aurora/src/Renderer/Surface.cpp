#include "Core/Logging.h"
#include "Aurora/Renderer/Surface.h"
#include "Aurora/Renderer/APIType.h"
#include "Renderer/VulkanSpecifics/VulkanSurface.h"

namespace Aurora
{
	std::shared_ptr<Surface> Surface::Create(const SurfaceSpecification& specs)
	{
		switch (specs.API)
		{
			case APIType::API_TYPE_VULKAN : return std::make_shared<Aurora::VK::VulkanSurface>(specs);
			default:
			{
				AURORA_ERROR("Chosen API ({}) not supported! Returning nullptr", static_cast<uint32_t>(specs.API));
				return nullptr;
			}
		}
	}

}
