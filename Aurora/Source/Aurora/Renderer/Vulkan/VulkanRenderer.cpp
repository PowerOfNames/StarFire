#include "Aurora/Renderer/Vulkan/VulkanRenderer.h"
#include "Aurora/Renderer/Vulkan/VulkanDebug.h"
#include "Aurora/Renderer/Vulkan/Utility/VulkanConvert.h"
#include "Aurora/Renderer/Vulkan/VulkanGraphicsPipeline.h"

namespace Aurora::VK {
	bool VulkanRenderer::Init()
	{
		PROFILE_FUNCTION;

		AURORA_INFO("Initializing VulkanRenderer...");
		
		return true;
	}

	void VulkanRenderer::Destroy()
	{
		PROFILE_FUNCTION;

		AURORA_INFO("Destroying VulkanRenderer...");
	}


	void VulkanRenderer::BindPipeline(VkCommandBuffer cmd, Ref<GraphicsPipeline> pipeline)
	{
		PROFILE_FUNCTION;

		Ref<VulkanGraphicsPipeline> vkPipeline = pipeline.As<VulkanGraphicsPipeline>();

		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline->GetPipelineHandle());
		VkBindDescriptorSetsInfo setInfo{ VK_STRUCTURE_TYPE_BIND_DESCRIPTOR_SETS_INFO };
		setInfo.pNext = nullptr;
		VkDescriptorSet descSet = vkPipeline->GetDescriptorSet();
		setInfo.descriptorSetCount = 1;
		setInfo.pDescriptorSets = &descSet;
		setInfo.dynamicOffsetCount = 0;
		setInfo.firstSet = 0;
		setInfo.layout = vkPipeline->GetPipelineLayoutHandle();
		setInfo.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		vkCmdBindDescriptorSets2(cmd, &setInfo);
	}

	void VulkanRenderer::Draw(VkCommandBuffer cmd, uint32_t vertexCount)
	{
		PROFILE_FUNCTION;

		if (vertexCount == 0)
			return;

		vkCmdDraw(cmd, vertexCount, 1, 0, 0);
	}

	void VulkanRenderer::DrawIndexed(VkCommandBuffer cmd, const IndexedDrawData& drawData)
	{
		PROFILE_FUNCTION;

		if (drawData.IndexCount == 0)
			return;

		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(cmd, 0, 1, &drawData.VertexBuffer, offsets);
		vkCmdBindIndexBuffer(cmd, drawData.IndexBuffer, drawData.IndexOffset, Convert::ToVkIndexType(drawData.IndexType));
		vkCmdDrawIndexed(cmd, drawData.IndexCount, drawData.InstanceCount, drawData.FirstIndex, drawData.VertexOffset, drawData.FirstInstance);
	}
}