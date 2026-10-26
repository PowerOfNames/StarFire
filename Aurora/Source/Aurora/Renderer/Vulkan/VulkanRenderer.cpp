#include "Aurora/Renderer/Vulkan/VulkanRenderer.h"
#include "Aurora/Renderer/Vulkan/VulkanDebug.h"
#include "Aurora/Renderer/Vulkan/Utility/VulkanToString.h"
#include "Aurora/Renderer/Vulkan/Utility/VulkanConvert.h"
#include "Aurora/Renderer/Vulkan/VulkanGraphicsPipeline.h"
#include "Aurora/ShaderDataTypes.h"

#include "AuroraInternal.h"

namespace Aurora::VK {
	bool VulkanRenderer::Init()
	{
		PROFILE_FUNCTION;

		AURORA_INFO("Initializing VulkanRenderer...");
		

		if(AURORA_REQUIRE_FAILS(CreateDefaultSamplers(), "Failed to create default samplers"))
		   return false;

		if (AURORA_REQUIRE_FAILS(CreateBindlessHandles(), "Failed to create bindless handles."))
			return false;

		if (AURORA_REQUIRE_FAILS(InitializeBindlessImageSlots(), "Failed to create bindless image slot pool."))
			return false;

		return true;
	}

	void VulkanRenderer::Destroy()
	{
		PROFILE_FUNCTION;

		AURORA_INFO("Destroying VulkanRenderer...");

		Ref<VulkanContext> context = GetRenderContext();
		Ref<VulkanResourceManager> resManager = GetResourceManager();
		m_BindlessImageSlots.erase(m_FallbackTexture);
		resManager->DestroyImage(m_FallbackTexture);

		AURORA_VALIDATE(m_BindlessImageSlots.empty(), "There are still textures registered in bindless texture descriptor.");
		m_BindlessImageSlots.clear();
		m_FreeCount = MAX_IMAGE_REGISTER_COUNT;

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

		for (VkSampler sampler : m_DefaultSamplers)
		{
			context->SubmitToMainDeletionQueue(
			[
				sampler = sampler
			] (VkDevice device, VmaAllocator, const VkAllocationCallbacks* allocCbs)
			{
				vkDestroySampler(device, sampler, allocCbs);
			});
		}
		m_DefaultSamplers.clear();
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

	bool VulkanRenderer::CreateDefaultSamplers()
	{
		PROFILE_FUNCTION;

		VkDevice device = GetRenderContext()->GetLogicalDevice();
		const VkAllocationCallbacks* allocCbs = GetRenderContext()->GetAllocationCallbacks();

		m_DefaultSamplers.resize(1);
		std::string samplerName = "Sampler";
		// == Texture Sampler ==
		{
			VkSamplerCreateInfo createInfo{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
			createInfo.pNext = nullptr;
			createInfo.flags = 0;
			createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			createInfo.anisotropyEnable = false;
			createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			createInfo.compareEnable = false;
			createInfo.compareOp = VK_COMPARE_OP_EQUAL;
			createInfo.magFilter = VK_FILTER_LINEAR;
			createInfo.maxAnisotropy = 1.0f;
			createInfo.maxLod = 0;
			createInfo.minFilter = VK_FILTER_LINEAR;
			createInfo.minLod = 0;
			createInfo.mipLodBias = 0;
			createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			createInfo.unnormalizedCoordinates = false;

			VkSampler sampler = VK_NULL_HANDLE;
			samplerName = "Sampler_REPEAT";
			AURORA_VK_CHECK(vkCreateSampler(device, &createInfo, allocCbs, &sampler), VK_SUCCESS, "Failed to create sampler");
			if (AURORA_REQUIRE_FAILS(sampler != VK_NULL_HANDLE, "Failed to create sampler '{}'", samplerName))
				return false;
			AURORA_VK_ATTACH_DEBUG_NAME(device, VK_OBJECT_TYPE_SAMPLER, (uint64_t)sampler, samplerName);
			m_DefaultSamplers[AURORA_GPU_SAMPLERS_REPEAT_ID] = sampler;
		}

		return true;
	}

	bool VulkanRenderer::CreateBindlessHandles()
	{
		PROFILE_FUNCTION;
		
		AURORA_INFO("Creating bindless descriptor set...");
		
		Ref<VulkanContext> context = GetRenderContext();
		VkDevice device = context->GetLogicalDevice();
		const PhysicalDeviceLimits& limits = context->GetPhysicalDeviceLimits();
		const VkAllocationCallbacks* allocCallbacks = context->GetAllocationCallbacks();

		uint32_t defaultSamplerCount = static_cast<uint32_t>(m_DefaultSamplers.size());
		if (AURORA_REQUIRE_FAILS(defaultSamplerCount <= limits.DescriptorLimits.MaxPerStageSamplers, "Too many default samplers!"))
			return false;

		uint32_t sampledImagesCountReserve = AURORA_MAX_IMAGE_ALLOCATIONS;
		if(AURORA_REQUIRE_FAILS(sampledImagesCountReserve <= limits.DescriptorLimits.MaxPerStageDescriptorSampledImages, "Too many sampled images"))
			return false;

		// == Descriptor Pool ==
		std::vector<VkDescriptorPoolSize> poolSizes =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, defaultSamplerCount },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, sampledImagesCountReserve }
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
		samplerBinding.binding = AURORA_GPU_SAMPLER_BINDING;
		samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		samplerBinding.descriptorCount = static_cast<uint32_t>(m_DefaultSamplers.size());
		samplerBinding.pImmutableSamplers = m_DefaultSamplers.data();

		VkDescriptorSetLayoutBinding imageBinding{};
		imageBinding.binding = AURORA_GPU_IMAGE_BINDING;
		imageBinding.descriptorCount = sampledImagesCountReserve;
		imageBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		imageBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		std::vector<VkDescriptorSetLayoutBinding> bindings = { samplerBinding, imageBinding };

		VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO };
		bindingFlagsInfo.pNext = nullptr;
		VkDescriptorBindingFlags flags = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;
		std::vector<VkDescriptorBindingFlags> bindingFlags = { 0, flags };
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


	// ========== Texture Slots ==========
	bool VulkanRenderer::InitializeBindlessImageSlots()
	{
		PROFILE_FUNCTION;

		// slots n-1 .. 1, 0 reserved for fallback image
		for (uint32_t i = 0; i < m_BindlessImageSlotsFreeList.size(); i++)		
			m_BindlessImageSlotsFreeList[i] = m_BindlessImageSlotsFreeList.size() - i; // skip zero (fallback)
		m_FreeCount = MAX_IMAGE_REGISTER_COUNT;

		if (AURORA_REQUIRE_FAILS(CreateFallbackTexture(), "Failed to create fallback texture"))
			return false;

		return true;
	}

	bool VulkanRenderer::CreateFallbackTexture()
	{
		PROFILE_FUNCTION;

		Aurora::ImageSpecification fallbackSpecs{};
		fallbackSpecs.Name = "CheckerBoard";
		fallbackSpecs.Data = &m_FallbackCheckerBoardData;
		fallbackSpecs.Size = sizeof(uint32_t) * 4;
		fallbackSpecs.Width = 2;
		fallbackSpecs.Height = 2;
		fallbackSpecs.Format = Aurora::Format::RGBA8_UNORM;
		fallbackSpecs.MemUsage = Aurora::MemoryUsage::GPU_ONLY;
		fallbackSpecs.Usage = Aurora::ImageUsageFlags::SAMPLED;
		fallbackSpecs.ForceImmediateUpload = true;
		Ref<VulkanResourceManager> resManager = GetResourceManager();
		m_FallbackTexture = resManager->CreateImage(fallbackSpecs);

		VulkanImageData* imageData = resManager->GetImageData(m_FallbackTexture);
		if (AURORA_REQUIRE_FAILS(imageData != nullptr, "Failed to create fallback texture"))
			return false;

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = imageData->ImageView;

		VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		write.pNext = nullptr;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		write.dstArrayElement = FALLBACK_IMAGE_SLOT_IDX; //Hardcoded to slot 0
		write.dstBinding = AURORA_GPU_IMAGE_BINDING;
		write.dstSet = m_BindlessSetData.DescriptorSet;
		write.pImageInfo = &imageInfo;
		vkUpdateDescriptorSets(GetRenderContext()->GetLogicalDevice(), 1, &write, 0, nullptr);

		return true;
	}

	bool VulkanRenderer::RegisterTexture(ImageHandle textureHandle, VkImageLayout sampledLayout/* = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL*/)
	{
		PROFILE_FUNCTION;

		if (textureHandle == m_FallbackTexture)
			return true;

		if (m_BindlessImageSlots.contains(textureHandle))
			return true;

		//TODO: this can be multithreaded later, so make slotting thread safe
		if (AURORA_REQUIRE_FAILS(m_FreeCount > 0, "No free texture slots."))
			return false;

		uint32_t slot = m_BindlessImageSlotsFreeList[--m_FreeCount];
		m_BindlessImageSlots[textureHandle] = slot;


		Ref<VulkanResourceManager> resManager = GetResourceManager();
		VulkanImageData* imageData = resManager->GetImageData(textureHandle);
		bool isValid = true;
		if (imageData == nullptr)	
		{
			AURORA_WARN("Texture handle {} is invalid.", static_cast<uint64_t>(textureHandle));
			isValid = false;			
		}
		if (isValid && !((imageData->Usage & ImageUsageFlags::SAMPLED) == ImageUsageFlags::SAMPLED))
		{
			AURORA_WARN("Texture handle {} usage is not supported ({}).", static_cast<uint64_t>(textureHandle), ImageUsageFlagsToString(Convert::ToVkImageUsageFlags(imageData->Usage)));
			isValid = false;
		}
		if (!isValid)
		{
			m_BindlessImageSlots.erase(textureHandle);
			m_BindlessImageSlotsFreeList[m_FreeCount++] = slot;
			return false;
		}

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = sampledLayout;
		imageInfo.imageView = imageData->ImageView;		

		VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		write.pNext = nullptr;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		write.dstArrayElement = slot;
		write.dstBinding = AURORA_GPU_IMAGE_BINDING;
		write.dstSet = m_BindlessSetData.DescriptorSet;
		write.pImageInfo = &imageInfo;
		vkUpdateDescriptorSets(GetRenderContext()->GetLogicalDevice(), 1, &write, 0, nullptr);
		
		return true;
	}

	void VulkanRenderer::UnregisterTexture(ImageHandle textureHandle)
	{
		PROFILE_FUNCTION;

		if (AURORA_REQUIRE_FAILS(textureHandle != m_FallbackTexture, "Its not allowed to unregister the fallback texture!"))
			return;		

		auto slot = m_BindlessImageSlots.find(textureHandle);
		if (slot == m_BindlessImageSlots.end())		
			return;		

		m_BindlessImageSlotsFreeList[m_FreeCount++] = slot->second;
		m_BindlessImageSlots.erase(slot->first);
	}

	uint32_t VulkanRenderer::GetTextureSlot(ImageHandle textureHandle) const
	{
		PROFILE_FUNCTION;

		auto slot = m_BindlessImageSlots.find(textureHandle);
		if (slot == m_BindlessImageSlots.end())
			return FALLBACK_IMAGE_SLOT_IDX;
		
		return slot->second;
	}
}