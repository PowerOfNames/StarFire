#include "Aurora/Renderer/Vulkan/ImGuiImpl/VulkanImGuiRenderer.h"

#include "AuroraInternal.h"
#include "Aurora/Profiling/Profiling.h"

#include "Aurora/Renderer/Vulkan/VulkanCore.h"
#include "Aurora/Renderer/Vulkan/VulkanContext.h"
#include "Aurora/Renderer/Vulkan/Utility/VulkanConvert.h"
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

		m_ImageFormat = Format::RGBA8_UNORM;
		std::vector<VkFormat> formats = { Convert::ToVkFormat(m_ImageFormat) };
		initInfo.PipelineInfoMain.PipelineRenderingCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
		initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount = static_cast<uint32_t>(formats.size());
		initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.pColorAttachmentFormats = formats.data();

		initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		initInfo.CheckVkResultFn = CheckVkResult;
		ImGui_ImplVulkan_Init(&initInfo);

		OnFramebufferResize(swapchain->GetExtent().width, swapchain->GetExtent().height);
	}

	void VulkanImGuiRenderer::OnFramebufferResize(uint32_t width, uint32_t height)
	{
		if (width == m_Width && height == m_Height)
			return;

		//size == null check redundant, it get caught by the event dispatch

		m_Width = width;
		m_Height = height;		
		m_NeedsResize = true;
	}

	void VulkanImGuiRenderer::Shutdown()
	{
		PROFILE_FUNCTION;


		AURORA_INFO("Shutting down ImGui-VulkanImpl...");

		Ref<VulkanContext> renderContext = GetRenderContext();
		VkDevice device = renderContext->GetLogicalDevice();
		Ref<VulkanResourceManager> resourceManager = GetResourceManager();
		const VkAllocationCallbacks* allocCbs = renderContext->GetAllocationCallbacks();
		AURORA_VK_CHECK(vkDeviceWaitIdle(device), VK_SUCCESS, "Failed to wait for device idle!");


		//TODO: make sure all textures get destroyed before
		for (auto& [imageHandle, textureID] : m_TextureIDMap)
		{
			ImGui_ImplVulkan_RemoveTexture((VkDescriptorSet)textureID);
		}
		m_TextureIDMap.clear();

		vkDestroySampler(device, m_TextureSampler, allocCbs);
		ImGui_ImplVulkan_Shutdown();
		vkDestroyDescriptorPool(device, m_DescriptorPool, allocCbs);

		for (size_t fif = 0; fif < m_RenderTargets.size(); fif++)
			resourceManager->DestroyImage(m_RenderTargets[fif]);
		m_RenderTargets.clear();
	}

	void VulkanImGuiRenderer::BeginFrame()
	{
		PROFILE_FUNCTION;

		ImGui_ImplVulkan_NewFrame();


		Ref<VulkanResourceManager> resourceManager = GetResourceManager();
		std::vector<ImageHandle> invalidHandles;
		for (const auto& [imageHandle, descSet] : m_TextureIDMap)
		{
			if (resourceManager->IsHandleValid(imageHandle))
				continue;
			invalidHandles.push_back(imageHandle);
		}
		for(const auto& handle : invalidHandles)
			ReturnTextureIDFromHandle(handle);

		if (!m_NeedsResize)
			return;
		

		Ref<VulkanContext> renderContext = GetRenderContext();

		for (size_t fif = 0; fif < m_RenderTargets.size(); fif++)
			resourceManager->DestroyImage(m_RenderTargets[fif]);
		m_RenderTargets.clear();

		m_RenderTargets.resize(renderContext->GetFramesInFlightCount());
		bool success = true;
		for (size_t i = 0; i < m_RenderTargets.size(); i++)
		{
			ImageSpecification imageSpecs{};
			imageSpecs.Name = "ImGui_RenderTarget_" + std::to_string(i);
			imageSpecs.Width = m_Width;
			imageSpecs.Height = m_Height;
			imageSpecs.Format = m_ImageFormat;
			imageSpecs.MemUsage = MemoryUsage::GPU_ONLY;
			imageSpecs.Usage = ImageUsageFlags::COLOR_ATTACHMENT | ImageUsageFlags::TRANSFER_SRC;
			imageSpecs.Tiling = ImageTiling::OPTIMAL;

			ImageHandle handle = resourceManager->CreateImage(imageSpecs);
			if (!resourceManager->IsHandleValid(handle))
			{
				AURORA_ERROR("Failed to create ImGui render target image for frame {}. This will likely cause a crash later on when trying to use it.", i);
				success = false;
				continue;
			}
			m_RenderTargets[i] = handle;
		}

		m_NeedsResize = !success;		
	}

	void VulkanImGuiRenderer::EndFrame()
	{
		PROFILE_FUNCTION;


		VulkanFrame& frame = GetRenderContext()->GetCurrentFrameData();

		uint8_t frameIdx = frame.FrameIndex;
		if (frameIdx >= m_RenderTargets.size())
		{
			AURORA_ERROR("Invalid frame index {} for ImGui render target.", frameIdx);
			return;
		}

		Ref<VulkanResourceManager> resourceManager = GetResourceManager();
		VulkanImageData* renderTarget = resourceManager->GetImageData(m_RenderTargets[frameIdx]);
		if (!renderTarget)
		{
			AURORA_ERROR("Failed to retrieve ImGui render target image data for frame {}. Unable to produce final rendering via ImGui.", frameIdx);
			return;
		}

		VkCommandBuffer cmd = frame.CommandBuffer;

		VkImageLayout renderingImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		if (renderTarget->Layout != renderingImageLayout)
		{
			renderTarget->Layout = Commands::TransitionImageLayout(cmd, renderTarget->Image, renderTarget->Format, renderTarget->Layout, renderingImageLayout);
		}

		VkClearValue* clear = nullptr;

		VkRenderingAttachmentInfo colorAttachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
		colorAttachment.pNext = nullptr;
		colorAttachment.imageView = renderTarget->ImageView;
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
		if (renderTarget->Layout != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		{
			renderTarget->Layout = VK::Commands::TransitionImageLayout(frame.CommandBuffer, renderTarget->Image, renderTarget->Format, renderTarget->Layout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		}

		if (frame.TargetLayout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			frame.TargetLayout = VK::Commands::TransitionImageLayout(frame.CommandBuffer, frame.TargetImage, swapchainImageFormat, frame.TargetLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, true);
		}

		VK::Commands::BlitImageToImage(frame.CommandBuffer, renderTarget->Image, renderTarget->Width, renderTarget->Height, frame.TargetImage, frame.Extent.width, frame.Extent.height);

		renderTarget->Layout = VK::Commands::TransitionImageLayout(frame.CommandBuffer, renderTarget->Image, renderTarget->Format, renderTarget->Layout, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		frame.TargetLayout = VK::Commands::TransitionImageLayout(frame.CommandBuffer, frame.TargetImage, swapchainImageFormat, frame.TargetLayout, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, true);
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
		GetRenderContext()->SubmitToFrameDeletionQueue(
			[set = descriptorSet](VkDevice, VmaAllocator, const VkAllocationCallbacks*)
			{
				ImGui_ImplVulkan_RemoveTexture(set);												  
			});
		m_TextureIDMap.erase(it);
	}
}