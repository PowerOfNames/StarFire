#include "Aurora/Renderer/GraphicsPipeline.h"
#include "Aurora/Renderer/Vulkan/VulkanGraphicsPipeline.h"


namespace Aurora {

	Ref<GraphicsPipeline> GraphicsPipeline::Create(const GraphicsPipelineSpecification& specs)
	{
		return CreateRef<VK::VulkanGraphicsPipeline>(specs);
	}
}