#include "Aurora/Core/Core.h"
#include "Aurora/Renderer/Swapchain.h"
#include "Aurora/Renderer/VulkanHelper.h"
#include "Aurora/Renderer/DataStructs/SwapchainSupportDetails.h"
#include "Shaders/ShaderByteCodes.h"

namespace Aurora::VK {	

	Swapchain::Swapchain(const SwapchainSpecification& spec)
		: m_Specification(spec)
	{
	}

	void Swapchain::Init()
	{
		if (!CreateSwapchain(m_Specification.InitialExtent.Width, m_Specification.InitialExtent.Height))
		{
			AURORA_TRACE("Failed to create swapchain.");
			return;
		}

		if (!CreateImageViews())
		{
			AURORA_TRACE("Failed to create swapchain image view.");
			return;
		}
				
		if (!CreateRenderPass())
		{
			AURORA_TRACE("Failed to create swapchain render pass handle.");
			return;
		}

		if (!CreateFramebuffers())
		{
			AURORA_TRACE("Failed to create swapchain framebuffers");
			return;
		}

		if (!AllocateCommandBuffers())
		{
			AURORA_TRACE("Failed to allocate command buffers");
			return;
		}

		if (!CreateSyncObjects())
		{
			AURORA_TRACE("Failed to create swapchain sync objects");
			return;
		}

		InitializeFrames();

		if (!CreateFallbackPipeline())
		{
			AURORA_TRACE("Failed to create fallabck pipeline");
			//we technically do not need this EO, but will do anyways for development purposes
			return;
		}
	}

	void Swapchain::CleanupSwapchain()
	{
		AURORA_TRACE("Cleaning up swapchain.");
		vkDeviceWaitIdle(m_Specification.Device);

		for (auto framebuffer : m_Framebuffers)
			vkDestroyFramebuffer(m_Specification.Device, framebuffer, nullptr);
		m_Framebuffers.clear();

		for (auto imageView : m_ImageViews)
			vkDestroyImageView(m_Specification.Device, imageView, nullptr);
		m_ImageViews.clear();

		vkDestroySwapchainKHR(m_Specification.Device, m_Swapchain, nullptr);
		m_Swapchain = VK_NULL_HANDLE;
		m_Images.clear();
		AURORA_TRACE("Cleaning swapchin finished.");
	}

	void Swapchain::Destroy()
	{
		vkDeviceWaitIdle(m_Specification.Device);

		for(auto sema : m_ImageAvailableSemaphores)
			vkDestroySemaphore(m_Specification.Device, sema, nullptr);
		m_ImageAvailableSemaphores.clear();

		for(auto sema : m_RenderFinishedSemaphores)
			vkDestroySemaphore(m_Specification.Device, sema, nullptr);
		m_RenderFinishedSemaphores.clear();

		for(auto fence : m_InFlightFences)
			vkDestroyFence(m_Specification.Device, fence, nullptr);	
		m_InFlightFences.clear();
		
		vkFreeCommandBuffers(m_Specification.Device, m_Specification.GraphicsCmdPool, static_cast<uint32_t>(m_CommandBuffers.size()), m_CommandBuffers.data());

		CleanupSwapchain();

		vkDestroyPipeline(m_Specification.Device, m_FallbackPipeline, nullptr);
		m_FallbackPipeline = VK_NULL_HANDLE;

		vkDestroyPipelineLayout(m_Specification.Device, m_FallbackPipelineLayout, nullptr);
		m_FallbackPipelineLayout = VK_NULL_HANDLE;

		vkDestroyRenderPass(m_Specification.Device, m_RenderPass, nullptr);
		m_RenderPass = VK_NULL_HANDLE;


		AURORA_INFO("Destroyed swapchain.");
	}
	
	bool Swapchain::PrepareFrame(uint32_t framesInFlightIdx)
	{
		m_FramesInFlightIdx = framesInFlightIdx;
		AURORA_TRACE("New FIF index: {}", m_FramesInFlightIdx);
		
		// EO, because this only happens if Present captured suboptimal but no resize event was triggered yet
		if (m_NeedsResize)
			return false;

		AcquireNextFrameData();
		if (m_NeedsResize)
			return false;

		m_FramesInFlight[m_FramesInFlightIdx].IsReady = true;

		return true;
	}

	void Swapchain::AcquireNextFrameData()
	{
		AURORA_TRACE("Acquire next image {}", m_FramesInFlightIdx);

		FrameData& frame = m_FramesInFlight[m_FramesInFlightIdx];

		vkWaitForFences(m_Specification.Device, 1, &m_InFlightFences[m_FramesInFlightIdx], VK_TRUE, UINT64_MAX);

		VkResult result = vkAcquireNextImageKHR(m_Specification.Device, m_Swapchain, UINT64_MAX, m_ImageAvailableSemaphores[m_FramesInFlightIdx], VK_NULL_HANDLE, &m_ImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			AURORA_ERROR("Swapchain not fitting. Failed image acquisition. Needs immediate resize.");
			m_NeedsResize = true;
			return;
		}
		else
		{
			AURORA_ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR, "Swapchain suboptimal. Require recreate.");
		}

		vkResetFences(m_Specification.Device, 1, &m_InFlightFences[m_FramesInFlightIdx]);
		vkResetCommandBuffer(m_CommandBuffers[m_FramesInFlightIdx], 0);

		frame.CommandBuffer = m_CommandBuffers[m_FramesInFlightIdx];
		frame.FrameIndex = m_FramesInFlightIdx;
		frame.FrameCount++;
		AURORA_TRACE("Acquired image {}", m_FramesInFlightIdx);
	}

	bool Swapchain::SwapImages()
	{				
		Submit();
		return Present();
	}

	void Swapchain::Submit()
	{
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.pNext = nullptr;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_FramesInFlight[m_FramesInFlightIdx].CommandBuffer;

		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &m_ImageAvailableSemaphores[m_FramesInFlightIdx]; //wait until image is available to render/draw to
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &m_RenderFinishedSemaphores[m_FramesInFlightIdx]; //signal when drawing is finished and ready to be presented
		AURORA_TRACE("Submitting frame {}", m_FramesInFlightIdx);

		AURORA_VK_CHECK(vkQueueSubmit(m_Specification.GraphicsQueue, 1, &submitInfo, m_InFlightFences[m_FramesInFlightIdx]), VK_SUCCESS, "Failed to submit draw render buffer!");
	}

	bool Swapchain::Present()
	{
		AURORA_TRACE("Presenting frame {}", m_FramesInFlightIdx);

		VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		presentInfo.pNext = nullptr;

		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_Swapchain;
		presentInfo.pWaitSemaphores = &m_RenderFinishedSemaphores[m_FramesInFlightIdx]; //wait until ready to be presented
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pImageIndices = &m_ImageIndex;
		presentInfo.pResults = nullptr;

		VkResult result;
		result = vkQueuePresentKHR(m_Specification.PresentQueue, &presentInfo);

		//check if the framebuffer resized
		if (result == VK_SUBOPTIMAL_KHR)
		{
			m_NeedsResize = true;
			AURORA_WARN("Swapchain not optimal.");
		}
		else if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			m_NeedsResize = true;
			AURORA_ERROR("Swapchain not usable. Presentation failed and resize required!");
			return false;
		}
		else
		{
			AURORA_ASSERT(result == VK_SUCCESS, "Failed to present swap chain image!");
		}
		AURORA_TRACE("Presented frame {}", m_FramesInFlightIdx);
		return true;
	}

	void Swapchain::OnResize(uint32_t width, uint32_t height)
	{
		if (m_Extent.width == width && m_Extent.height == height)
		{
			m_NeedsResize = false;
			return;
		}

		AURORA_TRACE("Resizing swapchain to [{}|{}]", width, height);
		CleanupSwapchain();

		if (!CreateSwapchain(width, height))
		{
			AURORA_TRACE("Failed to create(resize) swapchain.");
			return;
		}

		if (!CreateImageViews())
		{
			AURORA_TRACE("Failed to create(resize) swapchain image view.");
			return;
		}

		if (!CreateFramebuffers())
		{
			AURORA_TRACE("Failed to create(resize) swapchain framebuffers");
			return;
		}
		AURORA_INFO("Resized swapchain to [{}|{}]", width, height);

		m_NeedsResize = false;
	}

	bool Swapchain::CreateSwapchain(uint32_t width, uint32_t height)
	{
		SwapchainSupportDetails details = Helper::GetSwapSupportDetails(m_Specification.PhysicalDevice, m_Specification.Surface);

		VkSurfaceFormatKHR surfaceFormat = Helper::ChooseSwapSurfaceFormat(details.Formats, VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
		VkPresentModeKHR presentMode = Helper::ChooseSwapPresentMode(details.PresentModes, VK_PRESENT_MODE_MAILBOX_KHR);
		VkExtent2D extent = Helper::ChooseSwapExtent(details.Capabilities, width, height);

		uint32_t imageCount = details.Capabilities.minImageCount;
		if (details.Capabilities.maxImageCount > 0 && imageCount > details.Capabilities.maxImageCount)
			imageCount = details.Capabilities.maxImageCount;


		VkSwapchainCreateInfoKHR swapInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
		swapInfo.pNext = nullptr;
		swapInfo.flags = 0;
		swapInfo.surface = m_Specification.Surface;
		swapInfo.minImageCount = imageCount;
		swapInfo.imageFormat = surfaceFormat.format;
		swapInfo.imageColorSpace = surfaceFormat.colorSpace;
		swapInfo.presentMode = presentMode;
		swapInfo.imageExtent = extent;
		swapInfo.imageArrayLayers = 1; // for multi target rendering like VR, other 3D applications
		swapInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = Helper::FindQueueFamilies(m_Specification.PhysicalDevice, m_Specification.Surface);
		if (indices.SamePresentGraphics)
		{
			swapInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapInfo.queueFamilyIndexCount = 0;
			swapInfo.pQueueFamilyIndices = nullptr;
		}
		else
		{
			uint32_t neededIndices[] = { indices.Graphics, indices.Present };
			swapInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapInfo.queueFamilyIndexCount = 2;
			swapInfo.pQueueFamilyIndices = neededIndices;
		}
		swapInfo.preTransform = details.Capabilities.currentTransform; // no transformation
		swapInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapInfo.clipped = VK_TRUE;
		swapInfo.oldSwapchain = VK_NULL_HANDLE;

		AURORA_VK_CHECK(vkCreateSwapchainKHR(m_Specification.Device, &swapInfo, nullptr, &m_Swapchain), VK_SUCCESS, "Failed to create swapchain handle.");
		if (m_Swapchain == VK_NULL_HANDLE)		
			return false;		
		AURORA_VK_ATTACH_DEBUG_NAME(m_Specification.Device, VK_OBJECT_TYPE_SWAPCHAIN_KHR, (uint64_t)m_Swapchain, "Swapchain");

		m_Extent = extent;
		m_ImageFormat = surfaceFormat.format;

		return true;
	}

	bool Swapchain::CreateImageViews()
	{
		uint32_t swapImageCount;
		vkGetSwapchainImagesKHR(m_Specification.Device, m_Swapchain, &swapImageCount, nullptr);
		m_Images.resize(swapImageCount);
		vkGetSwapchainImagesKHR(m_Specification.Device, m_Swapchain, &swapImageCount, m_Images.data());

		m_ImageViews.resize(m_Images.size());
		VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		viewInfo.pNext = nullptr;
		viewInfo.flags = 0;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = m_ImageFormat;
		viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.layerCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.levelCount = 1;
		for (size_t i = 0; i < m_Images.size(); i++)
		{
			viewInfo.image = m_Images[i];

			AURORA_VK_CHECK(vkCreateImageView(m_Specification.Device, &viewInfo, nullptr, &m_ImageViews[i]), VK_SUCCESS, "Failed to create swapchain image view.");
			if (m_ImageViews[i] == VK_NULL_HANDLE)
				return false;
			AURORA_VK_ATTACH_DEBUG_NAME(m_Specification.Device, VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)m_ImageViews[i], "Swapchain_ImageView");
		}
		return true;
	}

	bool Swapchain::CreateRenderPass()
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.flags = 0;
		colorAttachment.format = m_ImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorRef{};
		colorRef.attachment = 0;
		colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.flags = 0;
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorRef;
		subpass.inputAttachmentCount = 0;
		subpass.pInputAttachments = nullptr;
		subpass.pDepthStencilAttachment = nullptr;
		subpass.pPreserveAttachments = nullptr;
		subpass.preserveAttachmentCount = 0;
		subpass.pResolveAttachments = nullptr;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo passInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
		passInfo.pNext = nullptr;
		passInfo.flags = 0;
		passInfo.attachmentCount = 1;
		passInfo.pAttachments = &colorAttachment;
		passInfo.dependencyCount = 1;
		passInfo.pDependencies = &dependency;
		passInfo.subpassCount = 1;
		passInfo.pSubpasses = &subpass;

		AURORA_VK_CHECK(vkCreateRenderPass(m_Specification.Device, &passInfo, nullptr, &m_RenderPass), VK_SUCCESS, "Failed to create swapchain render pass");
		if (m_RenderPass == VK_NULL_HANDLE)
			return false;
		AURORA_VK_ATTACH_DEBUG_NAME(m_Specification.Device, VK_OBJECT_TYPE_RENDER_PASS, (uint64_t)m_RenderPass, "Swapchain_RenderPass");
		
		return true;
	}

	bool Swapchain::CreateFramebuffers()
	{
		m_Framebuffers.resize(m_ImageViews.size());
		for (size_t i = 0; i < m_ImageViews.size(); i++)
		{
			VkImageView attachments[] =
			{
				m_ImageViews[i]
			};

			VkFramebufferCreateInfo fbInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
			fbInfo.pNext = nullptr;
			fbInfo.flags = 0;
			fbInfo.attachmentCount = 1;
			fbInfo.pAttachments = attachments;
			fbInfo.height = m_Extent.height;
			fbInfo.width = m_Extent.width;
			fbInfo.layers = 1;
			fbInfo.renderPass = m_RenderPass;
			AURORA_VK_CHECK(vkCreateFramebuffer(m_Specification.Device, &fbInfo, nullptr, &m_Framebuffers[i]), VK_SUCCESS, "Failed to create swapchain framebuffer");
			if (m_Framebuffers[i] == VK_NULL_HANDLE)
				return false;
			AURORA_VK_ATTACH_DEBUG_NAME(m_Specification.Device, VK_OBJECT_TYPE_FRAMEBUFFER, (uint64_t)m_Framebuffers[i], "Swapchain_Framebuffer");
		}
		return true;
	}

	bool Swapchain::AllocateCommandBuffers()
	{
		m_CommandBuffers.resize(m_Specification.FramesInFlight);
		VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		allocInfo.pNext = nullptr;
		allocInfo.commandBufferCount = 1;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_Specification.GraphicsCmdPool;
		for (uint32_t i = 0; i < m_Specification.FramesInFlight; i++)
		{
			AURORA_VK_CHECK(vkAllocateCommandBuffers(m_Specification.Device, &allocInfo, &m_CommandBuffers[i]), VK_SUCCESS, "Failed to allocate swapchain command buffer.");
			if (m_CommandBuffers[i] == VK_NULL_HANDLE)
				return false;
			AURORA_VK_ATTACH_DEBUG_NAME(m_Specification.Device, VK_OBJECT_TYPE_COMMAND_BUFFER, (uint64_t)m_CommandBuffers[i], "Swapchain_commandBuffer_" + std::to_string(i));
		}
		return true;
	}

	bool Swapchain::CreateSyncObjects()
	{
		VkSemaphoreCreateInfo semaInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		semaInfo.pNext = nullptr;
		semaInfo.flags = VK_SEMAPHORE_TYPE_BINARY;
		
		VkFenceCreateInfo fenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		fenceInfo.pNext = nullptr;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		
		m_ImageAvailableSemaphores.resize(m_Specification.FramesInFlight);
		m_RenderFinishedSemaphores.resize(m_Specification.FramesInFlight);
		m_InFlightFences.resize(m_Specification.FramesInFlight);
		for (uint32_t i = 0; i < m_Specification.FramesInFlight; i++)
		{
			AURORA_VK_CHECK(vkCreateSemaphore(m_Specification.Device, &semaInfo, nullptr, &m_ImageAvailableSemaphores[i]), VK_SUCCESS, "Failed to create image available semaphore.");
			AURORA_VK_CHECK(vkCreateSemaphore(m_Specification.Device, &semaInfo, nullptr, &m_RenderFinishedSemaphores[i]), VK_SUCCESS, "Failed to create render finished semaphore.");		
			if (m_ImageAvailableSemaphores[i] == VK_NULL_HANDLE || m_RenderFinishedSemaphores[i] == VK_NULL_HANDLE)
				return false;

			std::string availableName = "Swapchain_Sema_Ava_" + std::to_string(i);
			std::string renderFinName = "Swapchain_Sema_RenderFin_" + std::to_string(i);
			AURORA_VK_ATTACH_DEBUG_NAME(m_Specification.Device, VK_OBJECT_TYPE_SEMAPHORE, (uint64_t)m_ImageAvailableSemaphores[i], availableName.c_str());
			AURORA_VK_ATTACH_DEBUG_NAME(m_Specification.Device, VK_OBJECT_TYPE_SEMAPHORE, (uint64_t)m_RenderFinishedSemaphores[i], "Swapchain_Sema_RenderFin_" + std::to_string(i));
			
			std::string inFlightName = "Swapchain_Fence_" + std::to_string(i);
			AURORA_VK_CHECK(vkCreateFence(m_Specification.Device, &fenceInfo, nullptr, &m_InFlightFences[i]), VK_SUCCESS, "Failed to create in-flight fence.");
			if (m_InFlightFences[i] == VK_NULL_HANDLE)
				return false;
			AURORA_VK_ATTACH_DEBUG_NAME(m_Specification.Device, VK_OBJECT_TYPE_FENCE, (uint64_t)m_InFlightFences[i], inFlightName.c_str());		
		}
		return true;
	}

	bool Swapchain::InitializeFrames()
	{
		m_FramesInFlight.resize(m_Specification.FramesInFlight);
		return true;
	}	

	//========== Fallback ==========
	void Swapchain::RecordFallbackSwapchainRenderPass()
	{
		VkRenderPassBeginInfo rpInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		rpInfo.pNext = nullptr;
		rpInfo.renderPass = m_RenderPass;
		rpInfo.framebuffer = m_Framebuffers[m_ImageIndex];
		rpInfo.renderArea.extent = m_Extent;
		rpInfo.renderArea.offset = { 0,0 };
		VkClearValue clearColor = { {{m_Specification.ClearColor.R, m_Specification.ClearColor.G, m_Specification.ClearColor.B, 1.0f}} };
		rpInfo.clearValueCount = 1;
		rpInfo.pClearValues = &clearColor;

		VkCommandBuffer cmd = m_CommandBuffers[m_FramesInFlightIdx];
		vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_FallbackPipeline);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_Extent.width);
		viewport.height = static_cast<float>(m_Extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(cmd, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = m_Extent;
		vkCmdSetScissor(cmd, 0, 1, &scissor);

		vkCmdDraw(cmd, 3, 1, 0, 0);

		vkCmdEndRenderPass(cmd);
	}

	bool Swapchain::CreateFallbackPipeline()
	{
		//Shader Modules					
		VkShaderModuleCreateInfo vertInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
		vertInfo.pNext = nullptr;
		vertInfo.flags = 0;
		vertInfo.codeSize = Shaders::SwapchainFallback_vert_size;
		vertInfo.pCode = reinterpret_cast<const uint32_t*>(&Shaders::SwapchainFallback_vert);

		VkShaderModule vertModule = VK_NULL_HANDLE;
		AURORA_VK_CHECK(vkCreateShaderModule(m_Specification.Device, &vertInfo, nullptr, &vertModule), VK_SUCCESS, "Failed to create swapchain fallback vertex shader module.");
		if (vertModule == VK_NULL_HANDLE)
			return false;

		VkShaderModuleCreateInfo fragInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
		fragInfo.pNext = nullptr;
		fragInfo.flags = 0;
		fragInfo.codeSize = Shaders::SwapchainFallback_frag_size;
		fragInfo.pCode = reinterpret_cast<const uint32_t*>(&Shaders::SwapchainFallback_frag);

		VkShaderModule fragModule = VK_NULL_HANDLE;
		AURORA_VK_CHECK(vkCreateShaderModule(m_Specification.Device, &fragInfo, nullptr, &fragModule), VK_SUCCESS, "Failed to create swapchain fallback fragment shader module.");
		if (vertModule == VK_NULL_HANDLE)
			return false;
		

		//Shader stages
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };					
		vertShaderStageInfo.pNext = nullptr;
		vertShaderStageInfo.flags = 0;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertModule;
		vertShaderStageInfo.pName = "main";
		vertShaderStageInfo.pSpecializationInfo = nullptr;

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };			
		fragShaderStageInfo.pNext = nullptr;
		fragShaderStageInfo.flags = 0;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragModule;
		fragShaderStageInfo.pName = "main";
		fragShaderStageInfo.pSpecializationInfo = nullptr;

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };
		
		//Vertex input state
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		vertexInputInfo.pNext = nullptr;
		vertexInputInfo.flags = 0;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr;

		//Input assembly
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		inputAssemblyInfo.pNext = nullptr;
		inputAssemblyInfo.flags = 0;
		inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
		
		//Viewport
		// Not used (dynamic state)
		//VkViewport viewport{};
		//viewport.x = 0.0f;
		//viewport.y = 0.0f;
		//viewport.width = (float)m_Extent.width;
		//viewport.height = (float)m_Extent.height;
		//viewport.minDepth = 0.0f;
		//viewport.maxDepth = 1.0f;

		////Scissors
		//VkRect2D scissor{};
		//scissor.offset = { 0, 0 };
		//scissor.extent = m_Extent;

		//Danymic state
		std::vector<VkDynamicState> dynamicStates =
		{
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicStateInfo{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
		dynamicStateInfo.pNext = nullptr;
		dynamicStateInfo.flags = 0;
		dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicStateInfo.pDynamicStates = dynamicStates.data();

		VkPipelineViewportStateCreateInfo viewportInfo{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
		viewportInfo.pNext = nullptr;
		viewportInfo.flags = 0;
		viewportInfo.scissorCount = 1;
		viewportInfo.viewportCount = 1;

		//Rasterization state
		VkPipelineRasterizationStateCreateInfo rasterizationInfo{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
		rasterizationInfo.pNext = nullptr;
		rasterizationInfo.flags = 0;
		rasterizationInfo.depthClampEnable = VK_FALSE;
		rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
		rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationInfo.lineWidth = 1.0f;
		rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationInfo.depthBiasEnable = VK_FALSE;
		rasterizationInfo.depthBiasConstantFactor = 0.0f;
		rasterizationInfo.depthBiasClamp = 0.0f;
		rasterizationInfo.depthBiasSlopeFactor = 0.0f;

		//Multisampling state
		VkPipelineMultisampleStateCreateInfo multiSampInfo{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
		multiSampInfo.pNext = nullptr;
		multiSampInfo.flags = 0;
		multiSampInfo.sampleShadingEnable = VK_FALSE;
		multiSampInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multiSampInfo.minSampleShading = 1.0f;
		multiSampInfo.pSampleMask = nullptr;
		multiSampInfo.alphaToCoverageEnable = VK_FALSE;
		multiSampInfo.alphaToOneEnable = VK_FALSE;
		
		//DepthTesting (not used)
		//VkPipelineDepthStencilStateCreateInfo depthStencilInfo{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
		//depthStencilInfo.pNext = nullptr;
		//depthStencilInfo.flags = 0;
		//...

		//Color blending
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlendStateInfo{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
		colorBlendStateInfo.pNext = nullptr;
		colorBlendStateInfo.flags = 0;
		colorBlendStateInfo.logicOpEnable = VK_FALSE;
		colorBlendStateInfo.logicOp = VK_LOGIC_OP_COPY;
		colorBlendStateInfo.attachmentCount = 1;
		colorBlendStateInfo.pAttachments = &colorBlendAttachment;
		colorBlendStateInfo.blendConstants[0] = 0.0f;
		colorBlendStateInfo.blendConstants[1] = 0.0f;
		colorBlendStateInfo.blendConstants[2] = 0.0f;
		colorBlendStateInfo.blendConstants[3] = 0.0f;

		//Pipeline layout
		VkPipelineLayoutCreateInfo layoutInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
		layoutInfo.pNext = nullptr;
		layoutInfo.flags = 0;
		layoutInfo.setLayoutCount = 0;
		layoutInfo.pSetLayouts = nullptr;
		layoutInfo.pushConstantRangeCount = 0;
		layoutInfo.pPushConstantRanges = nullptr;
		
		AURORA_VK_CHECK(vkCreatePipelineLayout(m_Specification.Device, &layoutInfo, nullptr, &m_FallbackPipelineLayout), VK_SUCCESS, "Failed to create swapchain fallback pipeline layout.");
		if (m_FallbackPipelineLayout == VK_NULL_HANDLE)
			return false;
		AURORA_VK_ATTACH_DEBUG_NAME(m_Specification.Device, VK_OBJECT_TYPE_PIPELINE_LAYOUT, (uint64_t)m_FallbackPipelineLayout, "Swapchain_Fallback_PipelineLayout");

		//Pipeline
		VkGraphicsPipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
		pipelineInfo.pNext = nullptr;
		pipelineInfo.flags = 0;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
		pipelineInfo.pViewportState = &viewportInfo;
		pipelineInfo.pRasterizationState = &rasterizationInfo;
		pipelineInfo.pMultisampleState = &multiSampInfo;
		pipelineInfo.pDepthStencilState = nullptr;
		pipelineInfo.pColorBlendState = &colorBlendStateInfo;
		pipelineInfo.pDynamicState = &dynamicStateInfo;
		pipelineInfo.layout = m_FallbackPipelineLayout;
		pipelineInfo.renderPass = m_RenderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		AURORA_VK_CHECK(vkCreateGraphicsPipelines(m_Specification.Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_FallbackPipeline), VK_SUCCESS, "Failed to create swapchain fallback pipeline.");
		if (m_FallbackPipeline == VK_NULL_HANDLE)
			return false;
		AURORA_VK_ATTACH_DEBUG_NAME(m_Specification.Device, VK_OBJECT_TYPE_PIPELINE, (uint64_t)m_FallbackPipeline, "Swapchain_Fallback_Pipeline");

		vkDestroyShaderModule(m_Specification.Device, vertModule, nullptr);
		vkDestroyShaderModule(m_Specification.Device, fragModule, nullptr);

		return true;
	}

	

}