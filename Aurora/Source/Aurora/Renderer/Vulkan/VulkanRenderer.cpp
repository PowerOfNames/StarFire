#include "Aurora/Renderer/Vulkan/VulkanRenderer.h"
#include "Aurora/Renderer/Vulkan/VulkanDebug.h"
#include "Aurora/Renderer/Vulkan/Utility/VulkanConvert.h"
#include "Aurora/Renderer/Vulkan/VulkanGraphicsPipeline.h"
#include "Aurora/ShaderDataTypes.h"

#include "AuroraInternal.h"

namespace Aurora::VK {
	bool VulkanRenderer::Init()
	{
		PROFILE_FUNCTION;

		AURORA_INFO("Initializing VulkanRenderer...");
		

		if (AURORA_REQUIRE_FAILS(CreateBindlessHandles(), "Failed to create bindless handles."))
			return false;

		return true;
	}

	void VulkanRenderer::Destroy()
	{
		PROFILE_FUNCTION;

		AURORA_INFO("Destroying VulkanRenderer...");

		Ref<VulkanContext> context = GetRenderContext();

		context->SubmitToMainDeletionQueue(
			[
				descriptorPool = m_BindlessSetData.DescriptorPool,
				descriptorSetLayout = m_BindlessSetData.DescriptorSetLayout,
				pipelineLayout = m_BindlessSetData.PipelineLayout
			] (VkDevice device, VmaAllocator, const VkAllocationCallbacks* allocCbs)
			{
				vkDestroyDescriptorPool(device, descriptorPool, allocCbs);
				vkDestroyDescriptorSetLayout(device, descriptorSetLayout, allocCbs);
				vkDestroyPipelineLayout(device, pipelineLayout, allocCbs);
			});
		m_BindlessSetData.DescriptorPool = VK_NULL_HANDLE;
		m_BindlessSetData.DescriptorSetLayout = VK_NULL_HANDLE;
		m_BindlessSetData.PipelineLayout = VK_NULL_HANDLE;
	}


	void VulkanRenderer::BindPipeline(VkCommandBuffer cmd, Ref<GraphicsPipeline> pipeline) const
	{
		PROFILE_FUNCTION;

		Ref<VulkanGraphicsPipeline> vkPipeline = pipeline.As<VulkanGraphicsPipeline>();

		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline->GetPipelineHandle());
	}

	void VulkanRenderer::BindGlobalDescriptorSet(VkCommandBuffer cmd) const
	{
		PROFILE_FUNCTION;

		VkBindDescriptorSetsInfo setInfo{ VK_STRUCTURE_TYPE_BIND_DESCRIPTOR_SETS_INFO };
		setInfo.pNext = nullptr;
		VkDescriptorSet descSet = m_BindlessSetData.DescriptorSet;
		setInfo.descriptorSetCount = 1;
		setInfo.pDescriptorSets = &descSet;
		setInfo.dynamicOffsetCount = 0;
		setInfo.firstSet = 0;
		setInfo.layout = m_BindlessSetData.PipelineLayout;
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

		vkCmdBindIndexBuffer(cmd, drawData.IndexBuffer, drawData.IndexOffset, Convert::ToVkIndexType(drawData.IndexType));
		vkCmdDrawIndexed(cmd, drawData.IndexCount, drawData.InstanceCount, drawData.FirstIndex, drawData.VertexOffset, drawData.FirstInstance);
	}


	bool VulkanRenderer::CreateBindlessHandles()
	{
		PROFILE_FUNCTION;
		
		AURORA_INFO("Creating bindless descriptor set...");
		
		
		constexpr int SAMPLER_BINDING = 0;
		constexpr int IMAGE_BINDING = 1;


		Ref<VulkanContext> context = GetRenderContext();
		VkDevice device = context->GetLogicalDevice();
		const PhysicalDeviceLimits& limits = context->GetPhysicalDeviceLimits();
		const VkAllocationCallbacks* allocCallbacks = context->GetAllocationCallbacks();


		// == Descriptor Pool ==
		std::vector<VkDescriptorPoolSize> poolSizes =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, limits.DescriptorLimits.MaxPerStageSamplers },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, limits.DescriptorLimits.MaxPerStageDescriptorSampledImages }
		};

		VkDescriptorPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		poolInfo.pNext = nullptr;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
		poolInfo.maxSets = 1;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		AURORA_VK_CHECK(vkCreateDescriptorPool(device, &poolInfo, allocCallbacks, &m_BindlessSetData.DescriptorPool), VK_SUCCESS, "Failed to create bindless descriptor pool.");
		if (m_BindlessSetData.DescriptorPool == VK_NULL_HANDLE)
			return false;
		AURORA_VK_ATTACH_DEBUG_NAME(device, VK_OBJECT_TYPE_DESCRIPTOR_POOL, (uint64_t)m_BindlessSetData.DescriptorPool, "BindlessDescriptorPool");


		// == Descriptor Layout ==

		VkDescriptorSetLayoutBinding samplerBinding{};
		samplerBinding.binding = SAMPLER_BINDING;
		samplerBinding.descriptorCount = limits.DescriptorLimits.MaxPerStageSamplers;
		samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		samplerBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;

		VkDescriptorSetLayoutBinding imageBinding{};
		imageBinding.binding = IMAGE_BINDING;
		imageBinding.descriptorCount = limits.DescriptorLimits.MaxPerStageDescriptorSampledImages;
		imageBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		imageBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;

		std::vector<VkDescriptorSetLayoutBinding> bindings = { samplerBinding, imageBinding };

		VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO };
		bindingFlagsInfo.pNext = nullptr;
		VkDescriptorBindingFlags flags = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
		std::vector<VkDescriptorBindingFlags> bindingFlags = { flags, flags };
		bindingFlagsInfo.bindingCount = static_cast<uint32_t>(bindingFlags.size());
		bindingFlagsInfo.pBindingFlags = bindingFlags.data();


		VkDescriptorSetLayoutCreateInfo layoutInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		layoutInfo.pNext = &bindingFlagsInfo;
		layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		AURORA_VK_CHECK(vkCreateDescriptorSetLayout(device, &layoutInfo, allocCallbacks, &m_BindlessSetData.DescriptorSetLayout), VK_SUCCESS, "Failed to create descriptor set layout.");
		if (m_BindlessSetData.DescriptorSetLayout == VK_NULL_HANDLE)
			return false;
		AURORA_VK_ATTACH_DEBUG_NAME(device, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, (uint64_t)m_BindlessSetData.DescriptorSetLayout, "DescriptorSetLayout");

		// == Descriptor Set ==
		VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		allocInfo.pNext = nullptr;
		allocInfo.descriptorPool = m_BindlessSetData.DescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &m_BindlessSetData.DescriptorSetLayout;
		AURORA_VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, &m_BindlessSetData.DescriptorSet), VK_SUCCESS, "Failed to allocate bindless descriptor set.");
		if (m_BindlessSetData.DescriptorSet == VK_NULL_HANDLE)
			return false;
		AURORA_VK_ATTACH_DEBUG_NAME(device, VK_OBJECT_TYPE_DESCRIPTOR_SET, (uint64_t)m_BindlessSetData.DescriptorSet, "DescriptorSet");
		

		// == Pipeline Layout ==
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(GPU::BindlessPushConstant);
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkPipelineLayoutCreateInfo pipLayoutInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
		pipLayoutInfo.pNext = nullptr;
		pipLayoutInfo.flags = 0;
		pipLayoutInfo.setLayoutCount = 1;
		pipLayoutInfo.pSetLayouts = &m_BindlessSetData.DescriptorSetLayout;
		pipLayoutInfo.pushConstantRangeCount = 1;
		pipLayoutInfo.pPushConstantRanges = &pushConstantRange;

		AURORA_VK_CHECK(vkCreatePipelineLayout(device, &pipLayoutInfo, allocCallbacks, &m_BindlessSetData.PipelineLayout), VK_SUCCESS, "Failed to create bindless pipeline layout.");
		if (m_BindlessSetData.PipelineLayout == VK_NULL_HANDLE)
			return false;
		AURORA_VK_ATTACH_DEBUG_NAME(device, VK_OBJECT_TYPE_PIPELINE_LAYOUT, (uint64_t)m_BindlessSetData.PipelineLayout, "BindlessPipelineLayout");


		return true;
	}
}