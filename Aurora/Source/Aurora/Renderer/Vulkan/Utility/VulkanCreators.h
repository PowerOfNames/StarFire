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
	bool CreateImage(VmaAllocator allocator, VkImage* image, VmaAllocation* allocation, VkFormat format, VkImageUsageFlags usageFlags, VkImageTiling tiling, uint32_t width, uint32_t height, uint32_t mipLevels = 1);
	bool CreateImageView(VkDevice device, const VkAllocationCallbacks* allocationCbs, VkImageView* imageView, VkImage image, VkFormat format);

	// ========== Buffers ==========
	bool CreateBuffer(VmaAllocator allocator,
							 VkBuffer* buffer,
							 VmaAllocation* allocation,
							 VmaAllocationInfo* allocationInfo,
							 VkBufferUsageFlags usageFlags,
							 VmaMemoryUsage memUsage,
							 size_t size);
}
