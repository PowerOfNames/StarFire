#include "Aurora/Renderer/Vulkan/Utility/VulkanQueries.h"
#include "Aurora/Profiling/Profiling.h"

#include <vector>
#include <algorithm>


namespace Aurora::VK::Queries {

	// ========== Device ==========
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice phDevice, VkSurfaceKHR surface)
	{
		PROFILE_FUNCTION;

		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(phDevice, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilyProps(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(phDevice, &queueFamilyCount, queueFamilyProps.data());

		// We try to find distinct Graphics, Present, Transfer and Compute Queues
		QueueFamilyIndices indices{};
		//Best case: combined
		bool foundBestPresent = false;
		bool foundUnified = false;

		//Best case: dedicated
		bool foundBestTransfer = false;
		bool foundBestCompute = false;

		int i = 0;
		//Look for UnifiedGraphics queue (with present, graphics and compute support (most integrated GPUs have that))
		for (const auto& family : queueFamilyProps)
		{
			//look until a family was found that supports both present and graphics
			if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT
				&& !foundBestPresent)
			{
				indices.Graphics = i;
			}
			VkBool32 presentSupport;
			vkGetPhysicalDeviceSurfaceSupportKHR(phDevice, i, surface, &presentSupport);
			if (presentSupport && !foundBestPresent)
			{
				indices.Present = i;
				foundBestPresent = family.queueFlags & VK_QUEUE_GRAPHICS_BIT;
			}

			if (!foundBestTransfer && family.queueFlags & VK_QUEUE_TRANSFER_BIT)
			{
				indices.Transfer = i;
				foundBestTransfer = !(family.queueFlags & VK_QUEUE_GRAPHICS_BIT) && !(family.queueFlags & VK_QUEUE_COMPUTE_BIT);
			}

			if (!foundBestCompute && family.queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				indices.Compute = i;
				foundBestCompute = !(family.queueFlags & VK_QUEUE_GRAPHICS_BIT);
			}

			//mainly for integrated chips
			if (!foundUnified
				&& family.queueFlags & VK_QUEUE_COMPUTE_BIT
				&& family.queueFlags & VK_QUEUE_TRANSFER_BIT
				&& foundBestPresent)
			{
				indices.Unified = i;
				indices.UnifiedCount = family.queueCount;
				foundUnified = true;
			}
			i++;
		}
		indices.SamePresentGraphics = foundBestPresent;
		indices.HasDedicatedTransfer = foundBestTransfer;
		indices.HasDedicatedCompute = foundBestCompute;

		return indices;
	}

	// ========== Swapchain ==========
	const SwapchainSupportDetails GetSwapSupportDetails(VkPhysicalDevice phDevice, VkSurfaceKHR surface)
	{
		PROFILE_FUNCTION;

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

	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats, VkFormat preferredFormat, VkColorSpaceKHR preferredColorSpace)
	{
		PROFILE_FUNCTION;

		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == preferredFormat && availableFormat.colorSpace == preferredColorSpace)
				return availableFormat;
		}
		return availableFormats[0];
	}

	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availableModes, VkPresentModeKHR preferred)
	{
		PROFILE_FUNCTION;

		for (const auto& availableMode : availableModes)
		{
			if (availableMode == preferred)
				return availableMode;
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t framebufferWidth, uint32_t framebufferHeight)
	{
		PROFILE_FUNCTION;

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
