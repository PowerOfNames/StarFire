#include "Renderer/VulkanSpecifics/Swapchain.h"
#include "Renderer/VulkanSpecifics/VulkanHelper.h"

#include <algorithm>

namespace Aurora::VK {

	Swapchain::Swapchain(const SwapchainSpecification& spec)
		: m_Specification(spec)
	{
	}

	void Swapchain::Init()
	{		
		if (m_Swapchain == VK_NULL_HANDLE)
		{
			AURORA_TRACE("Failed to create swapchain handle.");
			return;
		}

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
	}

	void Swapchain::Destroy()
	{
		vkDeviceWaitIdle(m_Specification.Device);

		for (auto framebuffer : m_SwapchainFramebuffers)
			vkDestroyFramebuffer(m_Specification.Device, framebuffer, nullptr);
		m_SwapchainFramebuffers.clear();

		vkDestroyRenderPass(m_Specification.Device, m_RenderPass, nullptr);
		m_RenderPass = VK_NULL_HANDLE;

		for (auto imageView : m_SwapchainImageViews)
			vkDestroyImageView(m_Specification.Device, imageView, nullptr);		
		m_SwapchainImageViews.clear();

		vkDestroySwapchainKHR(m_Specification.Device, m_Swapchain, nullptr);
		m_Swapchain = VK_NULL_HANDLE;
		m_SwapchainImages.clear();

		AURORA_INFO("Destroyed swapchain.");
	}
	
	bool Swapchain::CreateSwapchain()
	{
		SwapchainSupportDetails details = GetSupportDetails(m_Specification.PhysicalDevice, m_Specification.Surface);

		VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(details.Formats, VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
		VkPresentModeKHR presentMode = ChooseSwapPresentMode(details.PresentModes, VK_PRESENT_MODE_MAILBOX_KHR);
		VkExtent2D extent = ChooseSwapExtent(details.Capabilities, m_Specification.InitialExtent.Width, m_Specification.InitialExtent.Height);

		uint32_t imageCount = details.Capabilities.minImageCount;
		if (details.Capabilities.maxImageCount > 0 && imageCount > details.Capabilities.maxImageCount)
			imageCount = details.Capabilities.maxImageCount;


		VkSwapchainCreateInfoKHR swapInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
		swapInfo.pNext = nullptr;
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

		m_SwapchainExtent = extent;
		m_SwapchainImageFormat = surfaceFormat.format;

		return true;
	}

	bool Swapchain::CreateImageViews()
	{
		uint32_t swapImageCount;
		vkGetSwapchainImagesKHR(m_Specification.Device, m_Swapchain, &swapImageCount, nullptr);
		m_SwapchainImages.resize(swapImageCount);
		vkGetSwapchainImagesKHR(m_Specification.Device, m_Swapchain, &swapImageCount, m_SwapchainImages.data());

		m_SwapchainImageViews.resize(m_SwapchainImages.size());
		VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		viewInfo.pNext = nullptr;
		viewInfo.flags = 0;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = m_SwapchainImageFormat;
		viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.layerCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.levelCount = 1;
		for (size_t i = 0; i < m_SwapchainImages.size(); i++)
		{
			viewInfo.image = m_SwapchainImages[i];

			AURORA_VK_CHECK(vkCreateImageView(m_Specification.Device, &viewInfo, nullptr, &m_SwapchainImageViews[i]), VK_SUCCESS, "Failed to create swapchain image view.");
			if (m_SwapchainImageViews[i] == VK_NULL_HANDLE)
				return false;
			AURORA_VK_ATTACH_DEBUG_NAME(m_Specification.Device, VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)m_SwapchainImageViews[i], "Swapchain_ImageView");
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
		colorAttachment.format = m_SwapchainImageFormat;
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

		VkRenderPassCreateInfo passInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
		passInfo.pNext = nullptr;
		passInfo.flags = 0;
		passInfo.attachmentCount = 1;
		passInfo.pAttachments = &colorAttachment;
		passInfo.dependencyCount = 1;
		passInfo.pDependencies = nullptr;
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
		m_SwapchainFramebuffers.resize(m_SwapchainImageViews.size());
		for (size_t i = 0; i < m_SwapchainImageViews.size(); i++)
		{
			VkImageView attachments[] =
			{
				m_SwapchainImageViews[i]
			};

			VkFramebufferCreateInfo fbInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
			fbInfo.pNext = nullptr;
			fbInfo.flags = 0;
			fbInfo.attachmentCount = 1;
			fbInfo.pAttachments = attachments;
			fbInfo.height = m_SwapchainExtent.height;
			fbInfo.width = m_SwapchainExtent.width;
			fbInfo.layers = 1;
			fbInfo.renderPass = m_RenderPass;
			AURORA_VK_CHECK(vkCreateFramebuffer(m_Specification.Device, &fbInfo, nullptr, &m_SwapchainFramebuffers[i]), VK_SUCCESS, "Failed to create swapchain framebuffer");
			if (m_SwapchainFramebuffers[i] == VK_NULL_HANDLE)
				return false;
			AURORA_VK_ATTACH_DEBUG_NAME(m_Specification.Device, VK_OBJECT_TYPE_FRAMEBUFFER, (uint64_t)m_SwapchainFramebuffers[i], "Swapchain_Framebuffer");
		}
		return true;
	}

	const SwapchainSupportDetails Swapchain::GetSupportDetails(VkPhysicalDevice phDevice, VkSurfaceKHR surface)
	{
		SwapchainSupportDetails details{};

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phDevice, surface, &details.Capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(phDevice, surface, &formatCount, nullptr);
		if (formatCount != 0)
		{
			details.Formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(phDevice, surface, &formatCount, details.Formats.data());
		}
		
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(phDevice, surface, &presentModeCount, nullptr);
		if (presentModeCount != 0)
		{
			details.PresentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(phDevice, surface, &presentModeCount, details.PresentModes.data());
		}

		return details;
	}

	VkSurfaceFormatKHR Swapchain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats, VkFormat preferredFormat, VkColorSpaceKHR preferredColorSpace)
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == preferredFormat && availableFormat.colorSpace == preferredColorSpace)
				return availableFormat;
		}
		return availableFormats[0];
	}

	VkPresentModeKHR Swapchain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availableModes, VkPresentModeKHR preferred)
	{
		for (const auto& availableMode : availableModes)
		{
			if (availableMode == preferred)
				availableMode;
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D Swapchain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t framebufferWidth, uint32_t framebufferHeight)
	{
		if (capabilities.currentExtent.width != UINT32_MAX)
			return capabilities.currentExtent;
		
		VkExtent2D actualExtent =
		{
			std::clamp(framebufferWidth, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
			std::clamp(framebufferHeight, capabilities.minImageExtent.height, capabilities.maxImageExtent.height),
		};
		
		return actualExtent;
	}

}