#include "Aurora/Renderer/Vulkan/Utility/VulkanHelper.h"
#include "Aurora/Profiling/Profiling.h"
#include "Aurora/Renderer/Vulkan/Utility/VulkanToString.h"


#include <vector>
#include <algorithm>


namespace Aurora::VK::Helper {

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

	//========== Swapchain ==========
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
				availableMode;
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

	void TransitionImageLayout(
		VkCommandBuffer cmd, 
		VkImage image, 
		VkImageLayout oldLayout, 
		VkImageLayout newLayout, 
		uint32_t baseMipLevel /*= 0*/, 
		uint32_t levelCount /*= VK_REMAINING_MIP_LEVELS*/, 
		uint32_t baseArrayLayer /*= 0*/, 
		uint32_t layerCount /*= VK_REMAINING_ARRAY_LAYERS*/)
	{
		PROFILE_FUNCTION;


		VkImageMemoryBarrier2 barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
		barrier.pNext = nullptr;

		barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
		barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		barrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

		if(newLayout == VK_IMAGE_LAYOUT_UNDEFINED ||
			newLayout == VK_IMAGE_LAYOUT_PREINITIALIZED)
			newLayout = VK_IMAGE_LAYOUT_GENERAL;

		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;

		switch (oldLayout)
		{
			case VK_IMAGE_LAYOUT_UNDEFINED:
			{
				barrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
				barrier.srcAccessMask = 0;
				break;
			}
			case VK_IMAGE_LAYOUT_GENERAL:
			{
				barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
				barrier.srcAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
				break;
			}
			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			{
				barrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
				barrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;

				break;
			}
			case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
			{
				barrier.srcStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
				barrier.srcAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				break;
			}
			case VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL:
			{
				barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
				barrier.srcAccessMask = VK_ACCESS_2_MEMORY_READ_BIT;
				break;
			}
			case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
			{
				barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
				barrier.srcAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
				break;
			}
			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			{
				barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
				barrier.srcAccessMask = VK_ACCESS_2_MEMORY_READ_BIT;
				break;
			}
			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			{
				barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
				barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
				break;
			}
			default:
			{
				AURORA_WARN("Unsupported layout transition! Old layout: {0}, new layout: {1}", LayoutToString(oldLayout).c_str(), LayoutToString(newLayout).c_str());
				break;
			}
		}

		switch (newLayout)
		{
			case VK_IMAGE_LAYOUT_GENERAL:
			{
				barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
				barrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
				break;
			}
			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			{
				barrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
				barrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
				break;
			}
			case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
			{
				barrier.dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
				barrier.dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				break;
			}
			case VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL:
			{
				barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
				barrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT;
				break;
			}
			case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
			{
				barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
				barrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
				break;
			}
			//TODO: Transfer should use transfer queue and therefore should be handle differently with queue ownership transfer as well
			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			{
				barrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
				barrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT;
				break;
			}
			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			{
				barrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
				barrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
				break;
			}
			default:
			{
				AURORA_WARN("Unsupported layout transition! Old layout: {0}, new layout: {1}", LayoutToString(oldLayout).c_str(), LayoutToString(newLayout).c_str());
				break;
			}
		}


		//Queue ownership transfer should be skipped if the contents of the resource does not need to be preserved, or if the resource is used on the same queue family on both sides of the transfer.
		//TODO: support for queue family ownership transfer if src and dst queue families differ -> https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/vkspec.html#synchronization-queue-transfers
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		barrier.subresourceRange.aspectMask = (newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = baseMipLevel;
		barrier.subresourceRange.levelCount = levelCount;
		barrier.subresourceRange.baseArrayLayer = baseArrayLayer;
		barrier.subresourceRange.layerCount = layerCount;
		barrier.image = image;

		VkDependencyInfo dependencyInfo{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
		dependencyInfo.pNext = nullptr;
		dependencyInfo.imageMemoryBarrierCount = 1;
		dependencyInfo.pImageMemoryBarriers = &barrier;

		vkCmdPipelineBarrier2(cmd, &dependencyInfo);
	}

	void BlitImageToImage(
		VkCommandBuffer cmd,
		VkImage srcImage,
		uint32_t srcWidth,
		uint32_t srcHeight,
		VkImage dstImage,
		uint32_t dstWidth,
		uint32_t dstHeight,
		uint32_t mipLevels /*= 1*/,
		uint32_t baseArrayLayer /*= 0*/,
		uint32_t layerCount /*= 1*/)
	{
		PROFILE_FUNCTION;


		std::vector<VkImageBlit2> blitRegions(mipLevels);
		for (uint32_t i = 0; i < mipLevels; i++)
		{
			blitRegions[i].sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;
			blitRegions[i].pNext = nullptr;
			blitRegions[i].srcOffsets[0] = { 0, 0, 0 };
			blitRegions[i].srcOffsets[1] = { static_cast<int32_t>(srcWidth >> i), static_cast<int32_t>(srcHeight >> i), 1 };
			blitRegions[i].srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blitRegions[i].srcSubresource.baseArrayLayer = baseArrayLayer;
			blitRegions[i].srcSubresource.mipLevel = i;
			blitRegions[i].srcSubresource.layerCount = layerCount;
			blitRegions[i].dstOffsets[0] = { 0, 0, 0 };
			blitRegions[i].dstOffsets[1] = { static_cast<int32_t>(dstWidth >> i), static_cast<int32_t>(dstHeight >> i), 1 };
			blitRegions[i].dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blitRegions[i].dstSubresource.baseArrayLayer = baseArrayLayer;
			blitRegions[i].dstSubresource.mipLevel = i;
			blitRegions[i].dstSubresource.layerCount = layerCount;
		}

		VkBlitImageInfo2 blitInfo{ VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2 };
		blitInfo.pNext = nullptr;
		blitInfo.srcImage = srcImage;
		blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		blitInfo.dstImage = dstImage;
		blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		blitInfo.filter = VK_FILTER_LINEAR;
		blitInfo.regionCount = static_cast<uint32_t>(blitRegions.size());
		blitInfo.pRegions = blitRegions.data();

		vkCmdBlitImage2(cmd, &blitInfo);
	}

	VkImageAspectFlags GetAspectFlagsFromFormat(VkFormat format)
	{
		VkImageAspectFlags aspects = 0;
		switch (format)
		{
			// Color formats
			case VK_FORMAT_R8G8B8A8_SRGB:
			case VK_FORMAT_R8G8B8A8_UNORM:
			case VK_FORMAT_B8G8R8A8_SRGB:
			case VK_FORMAT_B8G8R8A8_UNORM: aspects |= VK_IMAGE_ASPECT_COLOR_BIT; break;
			// Depth formats
			case VK_FORMAT_D32_SFLOAT: aspects |= VK_IMAGE_ASPECT_DEPTH_BIT; break;
			// Depth + Stencil formats
			case VK_FORMAT_D32_SFLOAT_S8_UINT: aspects |= VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT; break;
			default: AURORA_WARN("Unhandles VK_FORMAT {} detected. Falling back to VK_IMAGE_ASPECT_COLOR_BIT", FormatToString(format).c_str());
		}
		return aspects;
	}
}