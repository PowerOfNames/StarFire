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
	}

	void Swapchain::Destroy()
	{
		vkDestroySwapchainKHR(m_Specification.Device, m_Swapchain, nullptr);
		m_Swapchain = VK_NULL_HANDLE;
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