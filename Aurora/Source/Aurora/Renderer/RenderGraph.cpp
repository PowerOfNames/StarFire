#include "Aurora/Renderer/RenderGraph.h"
#include "Aurora/Renderer/Vulkan/VulkanRenderGraph.h"


namespace Aurora {

	Ref<RenderGraph> RenderGraph::Create(const RenderGraphSpecification& specs)
	{
		return CreateRef<VK::VulkanRenderGraph>(specs);
	}

}