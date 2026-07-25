#include "Aurora/Renderer/Vulkan/ImGuiImpl/VulkanImGuiRenderer.h"

#include "AuroraInternal.h"
#include "Aurora/Profiling/Profiling.h"

#include "Aurora/Renderer/Vulkan/VulkanCore.h"
#include "Aurora/Renderer/Vulkan/VulkanContext.h"
#include "Aurora/Renderer/Vulkan/Utility/VulkanCreators.h"
#include "Aurora/Renderer/Vulkan/Utility/VulkanCommands.h"


#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>

namespace Aurora::VK {
	

	static void CheckVkResult(VkResult err)
	{
		AURORA_VK_CHECK(err, VK_SUCCESS, "Error in ImGui - Vulkan");
	}

	void VulkanImGuiRenderer::Init()
	{
		PROFILE_FUNCTION;


		Ref<VulkanContext> renderContext = GetRenderContext();
		if (renderContext == nullptr)
		{
			AURORA_ERROR("VulkanImGuiRenderer::Init: RenderContext is null. Cannot initialize ImGui Vulkan renderer.");
			return;
		}
		VkDevice device = renderContext->GetLogicalDevice();
		const VkAllocationCallbacks* allocCbs = renderContext->GetAllocationCallbacks();

		QueueFamilies queueFamilies = renderContext->GetQueueFamilies();
		QueueFamilyIndices queueuIndices = renderContext->GetQueueFamilyIndices();

		ImGui_ImplVulkan_InitInfo initInfo = {};
		initInfo.ApiVersion = renderContext->GetApplicationInfo().apiVersion;
		initInfo.Instance = renderContext->GetVulkanInstance();
		initInfo.PhysicalDevice = renderContext->GetPhysicalDevice();
		initInfo.Device = device;
		initInfo.QueueFamily = queueuIndices.Graphics;
		initInfo.Queue = queueFamilies.Graphics;

		// ===== Descriptors =====
		{
			VkDescriptorPoolSize poolSizes[] =
			{
				// we need IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE (8) per font atlas plus as many as additional calls done to ImGui_ImplVulkan_AddTexture()
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE + 1000 },
			};
			VkDescriptorPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
			poolInfo.pNext = nullptr;
			poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			poolInfo.maxSets = 0;
			for (VkDescriptorPoolSize& poolSize : poolSizes)
				poolInfo.maxSets += poolSize.descriptorCount;
			poolInfo.poolSizeCount = (uint32_t)IM_COUNTOF(poolSizes);
			poolInfo.pPoolSizes = poolSizes;
			AURORA_VK_CHECK(vkCreateDescriptorPool(device, &poolInfo, allocCbs, &m_DescriptorPool), VK_SUCCESS, "Failed to create ImGui Vulkan descriptor pool.");
		}

		// ===== Texture Samples =====
		VkSamplerCreateInfo samplerInfo{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT; // outside image bounds just use border color
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.minLod = -1000;
		samplerInfo.maxLod = 1000;
		samplerInfo.maxAnisotropy = 1.0f;
		AURORA_VK_CHECK(vkCreateSampler(device, &samplerInfo, allocCbs, &m_TextureSampler), VK_SUCCESS, "Failed to create ImGui Vulkan texture sampler.");

		initInfo.DescriptorPool = m_DescriptorPool;
		initInfo.DescriptorPoolSize = 0;
		Ref<VulkanSwapchain> swapchain = renderContext->GetSwapchain();
		initInfo.MinImageCount = swapchain->GetSupportDetails().Capabilities.minImageCount;
		initInfo.ImageCount = swapchain->GetImageCount();
		initInfo.Allocator = renderContext->GetAllocationCallbacks(); // Optional: set to a Vulkan allocation callbacks if you have one		
		initInfo.UseDynamicRendering = true;

		m_ImageFormat = VK_FORMAT_R8G8B8A8_UNORM;
		initInfo.PipelineInfoMain.PipelineRenderingCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
		initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
		initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.pColorAttachmentFormats = &m_ImageFormat;

		initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		initInfo.CheckVkResultFn = CheckVkResult;
		ImGui_ImplVulkan_Init(&initInfo);

		m_RenderTargets.resize(initInfo.ImageCount);
		OnWindowResize(swapchain->GetExtent().width, swapchain->GetExtent().height);
	}

	void VulkanImGuiRenderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		PROFILE_FUNCTION;

		Ref<VulkanContext> renderContext = GetRenderContext();
		VkDevice device = renderContext->GetLogicalDevice();
		VmaAllocator allocator = renderContext->GetVmaAllocator();
		const VkAllocationCallbacks* allocCbs = renderContext->GetAllocationCallbacks();
		AURORA_VK_CHECK(vkDeviceWaitIdle(device), VK_SUCCESS, "Failed to wait for device idle!");

		uint32_t i = 0;
		for (VulkanImageData& imageData : m_RenderTargets)
		{
			imageData.Width = width;
			imageData.Height = height;
			imageData.MipLevels = 1;
			imageData.Format = m_ImageFormat;
			imageData.Layout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageData.Tiling = VK_IMAGE_TILING_OPTIMAL;
			Creators::CreateImage(allocator, &(imageData.Image), &(imageData.Allocation), imageData.Format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, imageData.Tiling, width, height, imageData.MipLevels);

			const std::string iString = std::to_string(i);
			const std::string imageName = "ImGui_Image_" + iString;
			AURORA_VK_ATTACH_DEBUG_NAME(device, VK_OBJECT_TYPE_IMAGE, (uint64_t)imageData.Image, imageName);

			Creators::CreateImageView(device, allocCbs, &(imageData.ImageView), imageData.Image, imageData.Format);
			const std::string imageViewName = "ImGui_ImageView_" + iString;
			AURORA_VK_ATTACH_DEBUG_NAME(device, VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)imageData.ImageView, imageViewName);
			i++;
		}
	}

	void VulkanImGuiRenderer::Shutdown()
	{
		PROFILE_FUNCTION;


		AURORA_INFO("Shutting down ImGui-VulkanImpl...");

		Ref<VulkanContext> renderContext = GetRenderContext();
		VkDevice device = renderContext->GetLogicalDevice();
		
		//TODO: make sure all textures get destroyed before
		for (auto& [imageHandle, textureID] : m_TextureIDMap)
		{
			ImGui_ImplVulkan_RemoveTexture((VkDescriptorSet)textureID);
		}
		m_TextureIDMap.clear();
		vkDestroySampler(device, m_TextureSampler, renderContext->GetAllocationCallbacks());
		
		AURORA_VK_CHECK(vkDeviceWaitIdle(device), VK_SUCCESS, "ImGuiRenderer::Shutdown: Failed to wait for device idle!");
		ImGui_ImplVulkan_Shutdown();

		for (VulkanImageData& imageData : m_RenderTargets)
		{
			vkDestroyImageView(device, imageData.ImageView, renderContext->GetAllocationCallbacks());
			vkDestroyImage(device, imageData.Image, renderContext->GetAllocationCallbacks());
			vmaFreeMemory(renderContext->GetVmaAllocator(), imageData.Allocation);
		}
		m_RenderTargets.clear();

		vkDestroyDescriptorPool(device, m_DescriptorPool, renderContext->GetAllocationCallbacks());
	}

	void VulkanImGuiRenderer::BeginFrame()
	{
		PROFILE_FUNCTION;


		ImGui_ImplVulkan_NewFrame();
	}

	void VulkanImGuiRenderer::EndFrame()
	{
		PROFILE_FUNCTION;


		VulkanFrame& frame = GetRenderContext()->GetCurrentFrameData();
		uint8_t frameIdx = frame.FrameIndex;
		VulkanImageData& renderTarget = m_RenderTargets[frameIdx];
		VkCommandBuffer cmd = frame.CommandBuffer;

		VkImageLayout renderingImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		if (renderTarget.Layout != renderingImageLayout)
		{
			renderTarget.Layout = Commands::TransitionImageLayout(cmd, renderTarget.Image, renderTarget.Format, renderTarget.Layout, renderingImageLayout);
		}

		VkClearValue* clear = nullptr;

		VkRenderingAttachmentInfo colorAttachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
		colorAttachment.pNext = nullptr;
		colorAttachment.imageView = renderTarget.ImageView;
		colorAttachment.imageLayout = renderingImageLayout;
		colorAttachment.loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		if(clear)
			colorAttachment.clearValue = *clear;

		VkRenderingInfo renderingInfo{ VK_STRUCTURE_TYPE_RENDERING_INFO };
		renderingInfo.pNext = nullptr;
		renderingInfo.colorAttachmentCount = 1;
		renderingInfo.pColorAttachments = &colorAttachment;
		renderingInfo.pDepthAttachment = nullptr;
		renderingInfo.pStencilAttachment = nullptr;
		renderingInfo.renderArea.extent = frame.Extent;
		renderingInfo.renderArea.offset = { 0,0 };
		renderingInfo.layerCount = 1;
		renderingInfo.viewMask = 0;
		renderingInfo.flags = 0;

		vkCmdBeginRendering(cmd, &renderingInfo);

		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

		vkCmdEndRendering(cmd);

		const Ref<VulkanSwapchain>& swapchain = GetRenderContext()->GetSwapchain();
		VkFormat swapchainImageFormat = swapchain->GetImageFormat();
		if (renderTarget.Layout != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		{
			renderTarget.Layout = VK::Commands::TransitionImageLayout(frame.CommandBuffer, renderTarget.Image, renderTarget.Format, renderTarget.Layout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		}

		if (frame.TargetLayout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			frame.TargetLayout = VK::Commands::TransitionImageLayout(frame.CommandBuffer, frame.TargetImage, swapchainImageFormat, frame.TargetLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		}

		VK::Commands::BlitImageToImage(frame.CommandBuffer, renderTarget.Image, renderTarget.Width, renderTarget.Height, frame.TargetImage, frame.Extent.width, frame.Extent.height);

		renderTarget.Layout = VK::Commands::TransitionImageLayout(frame.CommandBuffer, renderTarget.Image, renderTarget.Format, renderTarget.Layout, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		frame.TargetLayout = VK::Commands::TransitionImageLayout(frame.CommandBuffer, frame.TargetImage, swapchainImageFormat, frame.TargetLayout, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	}

	VulkanImageData& VulkanImGuiRenderer::GetRenderTarget(uint32_t frameIdx)
	{
		PROFILE_FUNCTION;


		size_t renderTargetCount = m_RenderTargets.size();
		if (renderTargetCount == 0)
		{
			AURORA_ERROR("Tried to get ImGui render target but no render targets are available. This likely means that the ImGui renderer was not initialized correctly or that the swapchain does not have any images. Returning a reference to a dummy render target, but this will likely cause a crash later on when trying to use it.");
			static VulkanImageData dummyRenderTarget{};
			return dummyRenderTarget;
		}
		if (frameIdx >= renderTargetCount)
		{
			AURORA_ERROR("Tried to get ImGui render target with index {} but only {} render targets are available. Using index 0", frameIdx, renderTargetCount);
			frameIdx = 0;
		}		

		return m_RenderTargets[frameIdx];
	}


	uint64_t VulkanImGuiRenderer::GetTextureIDFromHandle(ImageHandle image)
	{
		PROFILE_FUNCTION;

		auto it = m_TextureIDMap.find(image);
		if (it != m_TextureIDMap.end())
		{
			return it->second;
		}

		Ref<VulkanContext> renderContext = GetRenderContext();
		VulkanImageData* imageData = GetResourceManager()->GetImageData(image);
		if (!imageData)
		{
			AURORA_ERROR("Failed to create ImGui texture for invalid image handle.");
			return 0;
		}

		VkDescriptorSet descriptorSet = ImGui_ImplVulkan_AddTexture(m_TextureSampler, imageData->ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		uint64_t textureID = std::bit_cast<uint64_t, VkDescriptorSet>(descriptorSet);
		m_TextureIDMap[image] = textureID;
		return textureID;
	}

	void VulkanImGuiRenderer::ReturnTextureIDFromHandle(ImageHandle image)
	{
		PROFILE_FUNCTION;

		auto it = m_TextureIDMap.find(image);
		if (it == m_TextureIDMap.end())
		{
			AURORA_ERROR("Tried to return ImGui texture ID for image handle {} but it was not found in the texture ID map. Ignoring this call.", static_cast<uint16_t>(image));
			return;
		}

		VkDescriptorSet descriptorSet = std::bit_cast<VkDescriptorSet, uint64_t>(it->second);
		ImGui_ImplVulkan_RemoveTexture(descriptorSet);
		m_TextureIDMap.erase(it);
	}
}