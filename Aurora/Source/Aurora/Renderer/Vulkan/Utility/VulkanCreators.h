#pragma once

// ============================================================================
//  Utility placement rule - first match wins:
//    1. takes a VkCommandBuffer?              -> VulkanCommands
//    2. creates/destroys a Vk/VMA object?     -> VulkanCreators  (this file)
//    3. interrogates a VkPhysicalDevice
//       or VkSurfaceKHR?                      -> VulkanQueries
//    4. otherwise, pure function of enums
//       and PODs                              -> VulkanConvert
//       ...returning a string for logging?    -> VulkanToString
//
//  This file: functions that bring a Vulkan or VMA object into existence.
//  If it does not call a vk*Create* / vma*Create* entry point, it does not
//  belong here.
//
//  If a bucket passes ~300 lines, split it by resource (Image/Buffer),
//  never by adding a table of contents.
// ============================================================================

#include "Aurora/Renderer/Vulkan/VulkanCore.h"


namespace Aurora::VK::Creators {

	// ========== Images ==========

	/// <summary>
	/// Creates the VkImage and its allocation from the description already in
	/// <paramref name="data"/>. Reads Format, Usage, Tiling, Width, Height and
	/// MipLevels; writes Image and Allocation.
	/// </summary>
	bool CreateImage(VmaAllocator allocator, VulkanImageData& data, VmaMemoryUsage memUsage);

	/// <summary>
	/// Creates the VkImageView for an image already created by CreateImage.
	/// Reads Image and Format; writes ImageView.
	/// </summary>
	bool CreateImageView(VkDevice device, const VkAllocationCallbacks* allocationCbs, VulkanImageData& data);

	// ========== Buffers ==========
	bool CreateBuffer(VmaAllocator allocator,
							 VkBuffer* buffer,
							 VmaAllocation* allocation,
							 VmaAllocationInfo* allocationInfo,
							 VkBufferUsageFlags usageFlags,
							 VmaMemoryUsage memUsage,
							 size_t size);
}
