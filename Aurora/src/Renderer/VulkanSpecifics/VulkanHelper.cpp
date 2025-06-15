#include "Renderer/VulkanSpecifics/VulkanHelper.h"

#include <vector>

namespace Aurora::VK::Helper {

	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice phDevice, VkSurfaceKHR surface)
	{
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
}