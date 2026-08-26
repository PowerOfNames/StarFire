#include "Aurora/Renderer/Vulkan/Utility/VulkanCommands.h"
#include "Aurora/Core/Logging.h"
#include "Aurora/Profiling/Profiling.h"
#include "Aurora/Renderer/Vulkan/Utility/VulkanConvert.h"
#include "Aurora/Renderer/Vulkan/Utility/VulkanToString.h"

#include <vector>


namespace Aurora::VK::Commands {

	// ========== Images ==========
	VkImageLayout TransitionImageLayout(
		VkCommandBuffer cmd,
		VkImage image,
		VkFormat format,
		VkImageLayout oldLayout,
		VkImageLayout newLayout,
		bool targetSwapchain /*= false*/,
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

		if (newLayout == VK_IMAGE_LAYOUT_UNDEFINED ||
			newLayout == VK_IMAGE_LAYOUT_PREINITIALIZED)
		{
			newLayout = VK_IMAGE_LAYOUT_GENERAL;
			AURORA_WARN("Invalid target layout. Fallback to general.");
		}

		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;

		switch (oldLayout)
		{
			case VK_IMAGE_LAYOUT_UNDEFINED:
			{
				barrier.srcStageMask = targetSwapchain ? SWAPCHAIN_WAIT_STAGE : VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
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
				barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
				barrier.srcAccessMask = VK_ACCESS_2_MEMORY_READ_BIT;
				break;
			}
			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			{
				barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
				barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
				break;
			}
			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			{
				barrier.srcStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
				barrier.srcAccessMask = VK_ACCESS_2_MEMORY_READ_BIT;
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
			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			{
				barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
				barrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT;
				break;
			}
			//TODO: Transfer should use transfer queue and therefore should be handle differently with queue ownership transfer as well
			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			{
				barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
				barrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT;
				break;
			}
			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			{
				barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
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

		barrier.subresourceRange.aspectMask = Convert::GetAspectFlagsFromFormat(format);
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
		return newLayout;
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

	// ========== Buffer barriers ==========
	VkBufferMemoryBarrier2 EmitReleaseBarrier(VkBuffer buffer,
											  VkDeviceSize offset,
											  VkDeviceSize size,
											  uint32_t srcQueueFamilyIndex,
											  uint32_t dstQueueFamilyIndex,
											  VkPipelineStageFlags2 srcStageMask,
											  VkAccessFlags2 srcAccessMask,
											  VkPipelineStageFlags2 dstStageMask)
	{
		VkBufferMemoryBarrier2 barrier{ VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2 };
		barrier.pNext = nullptr;
		barrier.buffer = buffer;
		barrier.offset = offset;
		barrier.size = size;
		barrier.srcQueueFamilyIndex = srcQueueFamilyIndex;
		barrier.dstQueueFamilyIndex = dstQueueFamilyIndex;

		barrier.srcStageMask = srcStageMask;
		barrier.srcAccessMask = srcAccessMask;
		barrier.dstStageMask = dstStageMask;
		barrier.dstAccessMask = 0;

		return barrier;
	}

	VkBufferMemoryBarrier2 EmitAcquireBarrier(VkBuffer buffer,
											  VkDeviceSize offset,
											  VkDeviceSize size,
											  uint32_t srcQueueFamilyIndex,
											  uint32_t dstQueueFamilyIndex,
											  VkPipelineStageFlags2 srcStageMask,
											  VkPipelineStageFlags2 dstStageMask,
											  VkAccessFlags2 dstAccessMask)
	{
		VkBufferMemoryBarrier2 barrier{ VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2 };
		barrier.pNext = nullptr;
		barrier.buffer = buffer;
		barrier.offset = offset;
		barrier.size = size;
		barrier.srcQueueFamilyIndex = srcQueueFamilyIndex;
		barrier.dstQueueFamilyIndex = dstQueueFamilyIndex;

		barrier.srcStageMask = srcStageMask;
		barrier.srcAccessMask = 0;
		barrier.dstStageMask = dstStageMask;
		barrier.dstAccessMask = dstAccessMask;

		return barrier;
	}

	// ========== Image barriers ==========
	VkImageMemoryBarrier2 EmitLayoutTransitionBarrier(VkImage image,
													  VkImageLayout oldLayout,
													  VkImageLayout newLayout,
													  VkImageSubresourceRange range,
													  VkPipelineStageFlags2 srcStageMask,
													  VkAccessFlags2 srcAccessMask,
													  VkPipelineStageFlags2 dstStageMask,
													  VkAccessFlags2 dstAccessMask)
	{
		VkImageMemoryBarrier2 barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
		barrier.pNext = nullptr;
		barrier.image = image;
		barrier.subresourceRange = range;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		barrier.srcStageMask = srcStageMask;
		barrier.srcAccessMask = srcAccessMask;
		barrier.dstStageMask = dstStageMask;
		barrier.dstAccessMask = dstAccessMask;
		return barrier;
	}

	VkImageMemoryBarrier2 EmitReleaseBarrier(VkImage image,
											 VkImageLayout oldLayout,
											 VkImageLayout newLayout,
											 VkImageSubresourceRange range,
											 uint32_t srcQueueFamilyIndex,
											 uint32_t dstQueueFamilyIndex,
											 VkPipelineStageFlags2 srcStageMask,
											 VkAccessFlags2 srcAccessMask,
											 VkPipelineStageFlags2 dstStageMask)
	{
		VkImageMemoryBarrier2 barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
		barrier.pNext = nullptr;
		barrier.image = image;
		barrier.subresourceRange = range;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = srcQueueFamilyIndex;
		barrier.dstQueueFamilyIndex = dstQueueFamilyIndex;

		barrier.srcStageMask = srcStageMask;
		barrier.srcAccessMask = srcAccessMask;
		barrier.dstStageMask = dstStageMask;
		barrier.dstAccessMask = 0;

		return barrier;
	}

	VkImageMemoryBarrier2 EmitAcquireBarrier(VkImage image,
											 VkImageLayout oldLayout,
											 VkImageLayout newLayout,
											 VkImageSubresourceRange range,
											 uint32_t srcQueueFamilyIndex,
											 uint32_t dstQueueFamilyIndex,
											 VkPipelineStageFlags2 srcStageMask,
											 VkPipelineStageFlags2 dstStageMask,
											 VkAccessFlags2 dstAccessMask)
	{
		VkImageMemoryBarrier2 barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
		barrier.pNext = nullptr;
		barrier.image = image;
		barrier.subresourceRange = range;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = srcQueueFamilyIndex;
		barrier.dstQueueFamilyIndex = dstQueueFamilyIndex;

		barrier.srcStageMask = srcStageMask;
		barrier.srcAccessMask = 0;
		barrier.dstStageMask = dstStageMask;
		barrier.dstAccessMask = dstAccessMask;

		return barrier;
	}
}
