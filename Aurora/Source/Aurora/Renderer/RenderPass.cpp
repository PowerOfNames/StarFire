#include "Aurora/Renderer/RenderPass.h"
#include "Aurora/Renderer/Vulkan/VulkanRenderPass.h"

namespace Aurora {



	Ref<RenderPass> RenderPass::Create(const RenderPassSpecification& specs)
	{
		return CreateRef<VK::VulkanRenderPass>(specs);
	}
}

