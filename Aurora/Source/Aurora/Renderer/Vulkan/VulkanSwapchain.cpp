#include "Aurora/Core/Core.h"
#include "Aurora/Profiling/Profiling.h"
#include "Aurora/Renderer/Vulkan/VulkanSwapchain.h"
#include "Aurora/Renderer/Vulkan/Utility/VulkanQueries.h"

namespace Aurora::VK {

	VulkanSwapchain::VulkanSwapchain(const SwapchainSpecification& spec)
		: m_Specification(spec)
	{
		PROFILE_FUNCTION;

	}

	Ref<VulkanSwapchain> VulkanSwapchain::Create(const SwapchainSpecification& spec)
	{
		return CreateRef<VulkanSwapchain>(spec);
	}

	bool VulkanSwapchain::Init()
	{
		PROFILE_FUNCTION;

		if (AURORA_REQUIRE_FAILS(CreateSwapchain(m_Specification.InitialExtent.Width, m_Specification.InitialExtent.Height), "Failed to create swapchain."))		
			return false;

		if (AURORA_REQUIRE_FAILS(CreateImageViews(), "Failed to create swapchain image view."))		
			return false;		

		if (AURORA_REQUIRE_FAILS(CreateImageSemaphores(), "Failed to create swapchain image semaphores"))		
			return false;		

		if (AURORA_REQUIRE_FAILS(CreateRenderPass(), "Failed to create swapchain render pass handle."))
			return false;		

		if (AURORA_REQUIRE_FAILS(CreateFramebuffers(), "Failed to create swapchain framebuffers"))
			return false;

		return true;
	}

	void VulkanSwapchain::CleanupSwapchain()
	{
		PROFILE_FUNCTION;

		AURORA_TRACE("Cleaning up swapchain.");
		vkDeviceWaitIdle(m_Specification.Device);

		for (auto framebuffer : m_Framebuffers)
			vkDestroyFramebuffer(m_Specification.Device, framebuffer, m_Specification.AllocationCallbacks);
		m_Framebuffers.clear();

		for (auto imageView : m_ImageViews)
			vkDestroyImageView(m_Specification.Device, imageView, m_Specification.AllocationCallbacks);
		m_ImageViews.clear();

		vkDestroySwapchainKHR(m_Specification.Device, m_Swapchain, m_Specification.AllocationCallbacks);
		m_Swapchain = VK_NULL_HANDLE;
		m_Images.clear();
		AURORA_TRACE("Cleaning swapchin finished.");
	}

	void VulkanSwapchain::Destroy()
	{
		PROFILE_FUNCTION;

		vkDeviceWaitIdle(m_Specification.Device);

		CleanupSwapchain();

		for (auto sema : m_ImageAvailableSemaphores)
			vkDestroySemaphore(m_Specification.Device, sema, m_Specification.AllocationCallbacks);
		m_ImageAvailableSemaphores.clear();

		for (auto sema : m_ImageRenderFinishedSemaphores)
			vkDestroySemaphore(m_Specification.Device, sema, m_Specification.AllocationCallbacks);
		m_ImageRenderFinishedSemaphores.clear();

		vkDestroyRenderPass(m_Specification.Device, m_RenderPass, m_Specification.AllocationCallbacks);
		m_RenderPass = VK_NULL_HANDLE;


		AURORA_INFO("Destroyed swapchain.");
	}

	bool VulkanSwapchain::PrepareFrame(VulkanFrame& frame)
	{
		PROFILE_FUNCTION;

		// Check for resize and try
		if (m_NeedsResize)
		{
			SwapchainSupportDetails details = Queries::GetSwapSupportDetails(m_Specification.PhysicalDevice, m_Specification.Surface);
			VkExtent2D extent = Queries::ChooseSwapExtent(details.Capabilities, m_Extent.width, m_Extent.height);
			if (extent.width == 0 || extent.height == 0)
				return false;			
			OnResize(extent.width, extent.height, true);
		}

		//Resize failed, try again next frame
		if (m_NeedsResize)
			return false;


		vkWaitForFences(m_Specification.Device, 1, &frame.InFlightFence, VK_TRUE, UINT64_MAX);

		frame.InPresentation = false;

		VkResult result = vkAcquireNextImageKHR(m_Specification.Device, m_Swapchain, 1'000'000'000 /*1sec*/, m_ImageAvailableSemaphores[frame.FrameIndex], VK_NULL_HANDLE, &m_ImageIndex);
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
		else if (result != VK_SUCCESS)
		{
			AURORA_ASSERT(false, "Failed to acquire swap chain image!");
			return false;
		}
		vkResetFences(m_Specification.Device, 1, &frame.InFlightFence);
		
		frame.TargetImage = m_Images[m_ImageIndex];
		frame.TargetView = m_ImageViews[m_ImageIndex];
		frame.Extent = m_Extent;
		frame.TargetLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		vkResetCommandBuffer(frame.CommandBuffer, 0);

		return true;
	}

	bool VulkanSwapchain::SwapImages(VulkanFrame& frame, TimelineSemaphore& graphicsSemaphore)
	{
		PROFILE_FUNCTION;

		
		VkCommandBufferSubmitInfo cmdInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO };
		cmdInfo.pNext = nullptr;
		cmdInfo.commandBuffer = frame.CommandBuffer;
		cmdInfo.deviceMask = 0;

		VkSemaphoreSubmitInfo waitSemaphoreInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO };
		waitSemaphoreInfo.pNext = nullptr;
		waitSemaphoreInfo.semaphore = m_ImageAvailableSemaphores[frame.FrameIndex];
		waitSemaphoreInfo.stageMask = SWAPCHAIN_WAIT_STAGE; //As long as we blit into the swapchian, this should be enough
		waitSemaphoreInfo.deviceIndex = 0;
		
		VkSemaphoreSubmitInfo waitGraphicsSubmitInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO };
		waitGraphicsSubmitInfo.pNext = nullptr;
		waitGraphicsSubmitInfo.semaphore = graphicsSemaphore.Semaphore;
		waitGraphicsSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		waitGraphicsSubmitInfo.deviceIndex = 0;
		waitGraphicsSubmitInfo.value = graphicsSemaphore.Value;

		VkSemaphoreSubmitInfo signalSemaphoreInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO };
		signalSemaphoreInfo.pNext = nullptr;
		signalSemaphoreInfo.semaphore = m_ImageRenderFinishedSemaphores[m_ImageIndex];
		signalSemaphoreInfo.stageMask = SWAPCHAIN_WAIT_STAGE; //As long as we blit into the swapchian, this should be enough
		signalSemaphoreInfo.deviceIndex = 0;

		VkSemaphoreSubmitInfo signalGraphicsSubmitInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO };
		signalGraphicsSubmitInfo.pNext = nullptr;
		signalGraphicsSubmitInfo.semaphore = graphicsSemaphore.Semaphore;
		signalGraphicsSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		signalGraphicsSubmitInfo.deviceIndex = 0;
		signalGraphicsSubmitInfo.value = ++graphicsSemaphore.Value;

		std::vector<VkSemaphoreSubmitInfo> waitSubmitInfos = { waitSemaphoreInfo, waitGraphicsSubmitInfo };
		std::vector<VkSemaphoreSubmitInfo> signalSubmitInfos = { signalSemaphoreInfo, signalGraphicsSubmitInfo };

		// ===== Submission =====
		VkSubmitInfo2 submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO_2 };
		submitInfo.pNext = nullptr;
		submitInfo.commandBufferInfoCount = 1;
		submitInfo.pCommandBufferInfos = &cmdInfo;

		submitInfo.waitSemaphoreInfoCount = static_cast<uint32_t>(waitSubmitInfos.size());
		submitInfo.pWaitSemaphoreInfos = waitSubmitInfos.data();

		submitInfo.signalSemaphoreInfoCount = static_cast<uint32_t>(signalSubmitInfos.size());
		submitInfo.pSignalSemaphoreInfos = signalSubmitInfos.data();
		AURORA_VK_CHECK(vkQueueSubmit2(m_Specification.GraphicsQueue, 1, &submitInfo, frame.InFlightFence), VK_SUCCESS, "Failed to submit draw render buffer!");

		frame.InPresentation = true;

		// ===== Presentation =====
		VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		presentInfo.pNext = nullptr;

		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_Swapchain;
		presentInfo.pWaitSemaphores = &m_ImageRenderFinishedSemaphores[m_ImageIndex]; //wait until ready to be presented
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
		else if (result != VK_SUCCESS)
		{
			AURORA_ASSERT(false, "Failed to present swap chain image!");
			return false;
		}

		return true;
	}

	void VulkanSwapchain::OnResize(uint32_t width, uint32_t height, bool force /*= false*/)
	{
		PROFILE_FUNCTION;


		if (m_Extent.width == width && m_Extent.height == height && !force)
		{
			m_NeedsResize = false;
			return;
		}

		AURORA_TRACE("Resizing swapchain to [{}|{}] (Forced: {})", width, height, force);
		CleanupSwapchain();

		if (AURORA_REQUIRE_FAILS(CreateSwapchain(width, height), "Failed to create(resize) swapchain."))		
			return;		

		if (AURORA_REQUIRE_FAILS(CreateImageViews(), "Failed to create(resize) swapchain image view."))
			return;		

		if (AURORA_REQUIRE_FAILS(CreateFramebuffers(), "Failed to create(resize) swapchain framebuffers"))
			return;
		
		AURORA_INFO("Resized swapchain to [{}|{}] (Forced: {})", width, height, force);

		m_NeedsResize = false;
	}

	bool VulkanSwapchain::CreateSwapchain(uint32_t width, uint32_t height)
	{
		PROFILE_FUNCTION;


		SwapchainSupportDetails details = Queries::GetSwapSupportDetails(m_Specification.PhysicalDevice, m_Specification.Surface);

		//VkSurfaceFormatKHR surfaceFormat = Queries::ChooseSwapSurfaceFormat(details.Formats, VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
		VkSurfaceFormatKHR surfaceFormat = Queries::ChooseSwapSurfaceFormat(details.Formats, VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
		VkPresentModeKHR presentMode = Queries::ChooseSwapPresentMode(details.PresentModes, VK_PRESENT_MODE_MAILBOX_KHR);
		VkExtent2D extent = Queries::ChooseSwapExtent(details.Capabilities, width, height);

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
		swapInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		QueueFamilyIndices indices = Queries::FindQueueFamilies(m_Specification.PhysicalDevice, m_Specification.Surface);
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

		AURORA_VK_CHECK(vkCreateSwapchainKHR(m_Specification.Device, &swapInfo, m_Specification.AllocationCallbacks, &m_Swapchain), VK_SUCCESS, "Failed to create swapchain handle.");
		if (m_Swapchain == VK_NULL_HANDLE)
			return false;
		AURORA_VK_ATTACH_DEBUG_NAME(m_Specification.Device, VK_OBJECT_TYPE_SWAPCHAIN_KHR, (uint64_t)m_Swapchain, "Swapchain");

		m_Extent = extent;
		m_ImageFormat = surfaceFormat.format;

		return true;
	}

	bool VulkanSwapchain::CreateImageViews()
	{
		PROFILE_FUNCTION;

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

	bool VulkanSwapchain::CreateImageSemaphores()
	{
		PROFILE_FUNCTION;

		VkSemaphoreCreateInfo semaInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		semaInfo.pNext = nullptr;
		semaInfo.flags = VK_SEMAPHORE_TYPE_BINARY;

		m_ImageAvailableSemaphores.resize(m_Specification.FramesInFlight);
		for (size_t i = 0; i < m_ImageAvailableSemaphores.size(); i++)
		{
			const std::string iString = std::to_string(i);

			AURORA_VK_CHECK(vkCreateSemaphore(m_Specification.Device, &semaInfo, m_Specification.AllocationCallbacks, &m_ImageAvailableSemaphores[i]), VK_SUCCESS, "Failed to create image available semaphore.");
			if (m_ImageAvailableSemaphores[i] == VK_NULL_HANDLE)
				return false;
			const std::string availableName = "Frame_Sema_Ava_" + iString;
			AURORA_VK_ATTACH_DEBUG_NAME(m_Specification.Device, VK_OBJECT_TYPE_SEMAPHORE, (uint64_t)m_ImageAvailableSemaphores[i], availableName.c_str());
		}

		m_ImageRenderFinishedSemaphores.resize(m_Images.size());
		for (size_t i = 0; i < m_ImageRenderFinishedSemaphores.size(); i++)
		{
			const std::string iString = std::to_string(i);

			AURORA_VK_CHECK(vkCreateSemaphore(m_Specification.Device, &semaInfo, m_Specification.AllocationCallbacks, &m_ImageRenderFinishedSemaphores[i]), VK_SUCCESS, "Failed to create render finished semaphore.");
			if (m_ImageRenderFinishedSemaphores[i] == VK_NULL_HANDLE)
				return false;
			const std::string renderFinName = "Image_Sema_RenderFin_" + iString;
			AURORA_VK_ATTACH_DEBUG_NAME(m_Specification.Device, VK_OBJECT_TYPE_SEMAPHORE, (uint64_t)m_ImageRenderFinishedSemaphores[i], "Swapchain_Sema_RenderFin_" + iString);
		}

		return true;
	}

	bool VulkanSwapchain::CreateRenderPass()
	{
		PROFILE_FUNCTION;

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

		AURORA_VK_CHECK(vkCreateRenderPass(m_Specification.Device, &passInfo, m_Specification.AllocationCallbacks, &m_RenderPass), VK_SUCCESS, "Failed to create swapchain render pass");
		if (m_RenderPass == VK_NULL_HANDLE)
			return false;
		AURORA_VK_ATTACH_DEBUG_NAME(m_Specification.Device, VK_OBJECT_TYPE_RENDER_PASS, (uint64_t)m_RenderPass, "Swapchain_RenderPass");

		return true;
	}

	bool VulkanSwapchain::CreateFramebuffers()
	{
		PROFILE_FUNCTION;

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
			AURORA_VK_CHECK(vkCreateFramebuffer(m_Specification.Device, &fbInfo, m_Specification.AllocationCallbacks, &m_Framebuffers[i]), VK_SUCCESS, "Failed to create swapchain framebuffer");
			if (m_Framebuffers[i] == VK_NULL_HANDLE)
				return false;
			AURORA_VK_ATTACH_DEBUG_NAME(m_Specification.Device, VK_OBJECT_TYPE_FRAMEBUFFER, (uint64_t)m_Framebuffers[i], "Swapchain_Framebuffer");
		}
		return true;
	}	
}