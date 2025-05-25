#include "Core/Core.h"
#include "Aurora/Renderer/RenderContext.h"

#include "Renderer/VulkanSpecifics/VulkanRenderContext.h"

namespace Aurora {

	Ref<RenderContext> RenderContext::Create(const RenderContextSpecification& specs)
	{
		switch (specs.API)
		{
			case APIType::API_TYPE_VULKAN: return std::make_shared<VK::VulkanRenderContext>(specs);
			default:
			{
				AURORA_ERROR("Chosen API ({}) not supported! Returning nullptr", static_cast<uint32_t>(specs.API));
				return nullptr;
			}
		}
	}
}
