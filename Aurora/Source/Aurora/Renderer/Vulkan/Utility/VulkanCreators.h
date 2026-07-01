#pragma once

#include "Aurora/Renderer/Vulkan/VulkanCore.h"


namespace Aurora::VK::Creators {
	

	// ========== Images ==========
	bool CreateImage(VmaAllocator allocator, VkImage* image, VmaAllocation* allocation, VkFormat format, VkImageUsageFlags usageFlags, uint32_t width, uint32_t height, uint32_t mipLevels = 1);
	bool CreateImageView(VkDevice device, const VkAllocationCallbacks* allocationCbs, VkImageView* imageView, VkImage image, VkFormat format);

	// ========== Buffers ==========
	bool CreateBuffer(VmaAllocator allocator, 
							 VkBuffer* buffer, 
							 VmaAllocation* allocation,
							 VmaAllocationInfo* allocationInfo, 
							 VkBufferUsageFlags usageFlags, 
							 VmaMemoryUsage memUsage, 
							 size_t size);
	
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
}
