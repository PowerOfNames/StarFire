#include "Renderer/VulkanSpecifics/Swapchain.h"
#include "Renderer/VulkanSpecifics/VulkanHelper.h"
#include "Renderer/VulkanSpecifics/DataStructs/SwapchainSupportDetails.h"

namespace Aurora::VK {

	Swapchain::Swapchain(const SwapchainSpecification& spec)
		: m_Specification(spec)
	{
	}

	void Swapchain::Init()
	{
		if (!CreateSwapchain())
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

		for (auto framebuffer : m_Framebuffers)
			vkDestroyFramebuffer(m_Specification.Device, framebuffer, nullptr);
		m_Framebuffers.clear();

		vkDestroyRenderPass(m_Specification.Device, m_RenderPass, nullptr);
		m_RenderPass = VK_NULL_HANDLE;

		for (auto imageView : m_ImageViews)
			vkDestroyImageView(m_Specification.Device, imageView, nullptr);		
		m_ImageViews.clear();

		vkDestroySwapchainKHR(m_Specification.Device, m_Swapchain, nullptr);
		m_Swapchain = VK_NULL_HANDLE;
		m_Images.clear();

		AURORA_INFO("Destroyed swapchain.");
	}
	
	const FrameData* Swapchain::AcquireNextFrame()
	{
		vkWaitForFences(m_Specification.Device, 1, &m_InFlightFences[m_FrameIndex], VK_TRUE, UINT64_MAX);
		vkResetFences(m_Specification.Device, 1, &m_InFlightFences[m_FrameIndex]);

		vkAcquireNextImageKHR(m_Specification.Device, m_Swapchain, UINT64_MAX, m_ImageAvailableSemaphores[m_FrameIndex], VK_NULL_HANDLE, &m_ImageIndex);

		vkResetCommandBuffer(m_CommandBuffers[m_FrameIndex], 0);
		m_FramesInFlight[m_FrameIndex].CommandBuffer = m_CommandBuffers[m_FrameIndex];
		m_FramesInFlight[m_FrameIndex].FrameIndex = m_FrameIndex;
		
		
		return &m_FramesInFlight[m_FrameIndex];
	}

	void Swapchain::SwapImages()
	{
		AURORA_INFO("Current frame: {}", m_TotalFrames);
		AURORA_INFO("Swapping frame {}", m_FrameIndex);
		
		//temp
		AcquireNextFrame();
		
		Submit();
		Present();
		m_FrameIndex = (m_FrameIndex + 1) % m_Specification.FramesInFlight;
		m_TotalFrames++;
		AURORA_INFO("New frame index: {}", m_FrameIndex);
	}

	void Swapchain::Submit()
	{
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.pNext = nullptr;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_FramesInFlight[m_FrameIndex].CommandBuffer;

		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &m_ImageAvailableSemaphores[m_FrameIndex]; //wait until image is available to render/draw to
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &m_RenderFinishedSemaphores[m_FrameIndex]; //signal when drawing is finished and ready to be presented

		AURORA_VK_CHECK(vkQueueSubmit(m_Specification.GraphicsQueue, 1, &submitInfo, m_InFlightFences[m_FrameIndex]), VK_SUCCESS, "Failed to submit draw render buffer!");
	}

	void Swapchain::Present()
	{
		VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		presentInfo.pNext = nullptr;

		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_Swapchain;
		presentInfo.pWaitSemaphores = &m_RenderFinishedSemaphores[m_FrameIndex]; //wait until ready to be presented
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pImageIndices = &m_ImageIndex;
		presentInfo.pResults = nullptr;

		VkResult result;
		result = vkQueuePresentKHR(m_Specification.PresentQueue, &presentInfo);

		//check if the framebuffer resized
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR/* || m_FramebufferResized*/)
		{
			//m_FramebufferResized = false;
			//Recreate(Application::Get()->GetWindow().GetWidth(), Application::Get()->GetWindow().GetHeight());
			AURORA_INFO("Swapchain not up-to-date.");
		}
		else
		{
			AURORA_ASSERT(result == VK_SUCCESS, "Failed to present swap chain image!");
		}
	}

	void Swapchain::OnResize(uint32_t width, uint32_t height)
	{
		AURORA_INFO("Resizing swapchain to [{}|{}]", width, height);
	}

	bool Swapchain::CreateSwapchain()
	{
		SwapchainSupportDetails details = Helper::GetSwapSupportDetails(m_Specification.PhysicalDevice, m_Specification.Surface);

		VkSurfaceFormatKHR surfaceFormat = Helper::ChooseSwapSurfaceFormat(details.Formats, VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
		VkPresentModeKHR presentMode = Helper::ChooseSwapPresentMode(details.PresentModes, VK_PRESENT_MODE_MAILBOX_KHR);
		VkExtent2D extent = Helper::ChooseSwapExtent(details.Capabilities, m_Specification.InitialExtent.Width, m_Specification.InitialExtent.Height);

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

	bool Swapchain::CreatePipeline()
	{



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

}