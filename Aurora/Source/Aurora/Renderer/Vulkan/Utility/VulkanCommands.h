#pragma once

// ============================================================================
//  Utility placement rule - first match wins:
//    1. takes a VkCommandBuffer?              -> VulkanCommands  (this file)
//    2. creates/destroys a Vk/VMA object?     -> VulkanCreators
//    3. interrogates a VkPhysicalDevice
//       or VkSurfaceKHR?                      -> VulkanQueries
//    4. otherwise, pure function of enums
//       and PODs                              -> VulkanConvert
//       ...returning a string for logging?    -> VulkanToString
//
//  This file: recording into a command buffer, plus the barrier descriptors
//  that recording consumes. Per-frame, hot-path work.
//
//  The Emit*Barrier functions do not take a VkCommandBuffer themselves - they
//  build the structs that get recorded - but they exist only to serve
//  recording, so they live with it rather than in Creators (they create no
//  Vulkan object).
//
//  If a bucket passes ~300 lines, split it by resource (Image/Buffer),
//  never by adding a table of contents.
// ============================================================================

#include "Aurora/Renderer/Vulkan/VulkanCore.h"

namespace Aurora::VK::Commands {

	// ========== Images ==========

	/// <returns>The layout actually applied, which may differ from newLayout if newLayout was unusable.</returns>
	VkImageLayout TransitionImageLayout(
		VkCommandBuffer cmd,
		VkImage image,
		VkFormat format,
		VkImageLayout oldLayout,
		VkImageLayout newLayout,
		uint32_t baseMipLevel = 0,
		uint32_t levelCount = VK_REMAINING_MIP_LEVELS,
		uint32_t baseArrayLayer = 0,
		uint32_t layerCount = VK_REMAINING_ARRAY_LAYERS
	);

	void BlitImageToImage(
		VkCommandBuffer cmd,
		VkImage srcImage,
		uint32_t srcWidth,
		uint32_t srcHeight,
		VkImage dstImage,
		uint32_t dstWidth,
		uint32_t dstHeight,
		uint32_t mipLevels = 1,
		uint32_t baseArrayLayer = 0,
		uint32_t layerCount = 1
	);

	// ========== Buffer barriers ==========

	VkBufferMemoryBarrier2 EmitReleaseBarrier(VkBuffer buffer,
											  VkDeviceSize offset,
											  VkDeviceSize size,
											  uint32_t srcQueueFamilyIndex,
											  uint32_t dstQueueFamilyIndex,
											  VkPipelineStageFlags2 srcStageMask,
											  VkAccessFlags2 srcAccessMask,
											  VkPipelineStageFlags2 dstStageMask);

	VkBufferMemoryBarrier2 EmitAcquireBarrier(VkBuffer buffer,
											  VkDeviceSize offset,
											  VkDeviceSize size,
											  uint32_t srcQueueFamilyIndex,
											  uint32_t dstQueueFamilyIndex,
											  VkPipelineStageFlags2 srcStageMask,
											  VkPipelineStageFlags2 dstStageMask,
											  VkAccessFlags2 dstAccessMask);

	// ========== Image barriers ==========

	VkImageMemoryBarrier2 EmitLayoutTransitionBarrier(VkImage image,
													  VkImageLayout oldLayout,
													  VkImageLayout newLayout,
													  VkImageSubresourceRange range,
													  VkPipelineStageFlags2 srcStageMask,
													  VkAccessFlags2 srcAccessMask,
													  VkPipelineStageFlags2 dstStageMask,
													  VkAccessFlags2 dstAccessMask);

	VkImageMemoryBarrier2 EmitReleaseBarrier(VkImage image,
											 VkImageLayout oldLayout,
											 VkImageLayout newLayout,
											 VkImageSubresourceRange range,
											 uint32_t srcQueueFamilyIndex,
											 uint32_t dstQueueFamilyIndex,
											 VkPipelineStageFlags2 srcStageMask,
											 VkAccessFlags2 srcAccessMask,
											 VkPipelineStageFlags2 dstStageMask);

	VkImageMemoryBarrier2 EmitAcquireBarrier(VkImage image,
											 VkImageLayout oldLayout,
											 VkImageLayout newLayout,
											 VkImageSubresourceRange range,
											 uint32_t srcQueueFamilyIndex,
											 uint32_t dstQueueFamilyIndex,
											 VkPipelineStageFlags2 srcStageMask,
											 VkPipelineStageFlags2 dstStageMask,
											 VkAccessFlags2 dstAccessMask);
}
