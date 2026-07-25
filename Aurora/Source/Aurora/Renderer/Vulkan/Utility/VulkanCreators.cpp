#include "Aurora/Renderer/Vulkan/Utility/VulkanCreators.h"
#include "Aurora/Core/Logging.h"

#include "Aurora/Profiling/Profiling.h"
#include "Aurora/Renderer/Vulkan/VulkanCore.h"
#include "Aurora/Renderer/Vulkan/Utility/VulkanHelper.h"

namespace Aurora::VK::Creators {

#pragma region Images
	bool CreateImage(VmaAllocator allocator, VkImage* image, VmaAllocation* allocation, VkFormat format, VkImageUsageFlags usageFlags, VkImageTiling tiling, uint32_t width, uint32_t height, uint32_t mipLevels/* = 1*/)
	{
		PROFILE_FUNCTION;

		if (width == 0 || height == 0)
		{
			AURORA_WARN("Unable to create an image with extent ({}; {})", width, height);
			return false;
		}

		if (mipLevels == 0)
		{
			AURORA_WARN("Unable to create an image with 0 mip levels");
			return false;
		}

		// TODO: refactor out
		VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_GPU_ONLY;

		VkImageCreateInfo imageCreateInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		imageCreateInfo.pNext = nullptr;
		imageCreateInfo.flags = 0;
		imageCreateInfo.format = format;
		imageCreateInfo.extent = { width, height, 1 };
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.mipLevels = mipLevels;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.tiling = tiling;
		imageCreateInfo.usage = usageFlags;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.arrayLayers = 1;

		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = memUsage;
		allocCreateInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VkResult result = vmaCreateImage(allocator, &imageCreateInfo, &allocCreateInfo, image, allocation, nullptr);
		AURORA_VK_CHECK(result, VK_SUCCESS, "Failed to create image!");

		return true;
	}

	bool CreateImageView(VkDevice device, const VkAllocationCallbacks* allocationCbs, VkImageView* imageView, VkImage image, VkFormat format)
	{
		PROFILE_FUNCTION;


		VkImageViewCreateInfo viewCreateInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		viewCreateInfo.pNext = nullptr;
		viewCreateInfo.flags = 0;
		viewCreateInfo.image = image;
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCreateInfo.format = format;
		viewCreateInfo.components = {
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY
		};
		viewCreateInfo.subresourceRange.aspectMask = Helper::GetAspectFlagsFromFormat(format);
		viewCreateInfo.subresourceRange.baseArrayLayer = 0;
		viewCreateInfo.subresourceRange.layerCount = 1;
		viewCreateInfo.subresourceRange.baseMipLevel = 0;
		viewCreateInfo.subresourceRange.levelCount = 1;

		AURORA_VK_CHECK(vkCreateImageView(device, &viewCreateInfo, allocationCbs, imageView), VK_SUCCESS, "Failed to create image view!");
		return true;
	}
#pragma endregion Images

	bool CreateBuffer(VmaAllocator allocator, VkBuffer* buffer, VmaAllocation* allocation, VmaAllocationInfo* allocInfo, VkBufferUsageFlags usageFlags, VmaMemoryUsage memUsage, size_t size)
	{
		PROFILE_FUNCTION;


		if (size == 0)
		{
			AURORA_WARN("Unable to create a buffer with size 0");
			return false;
		}

		VkBufferCreateInfo bufferCreateInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferCreateInfo.pNext = nullptr;
		bufferCreateInfo.flags = 0;
		bufferCreateInfo.size = size;
		bufferCreateInfo.usage = usageFlags;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = memUsage;
		allocCreateInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VkResult result = vmaCreateBuffer(allocator, &bufferCreateInfo, &allocCreateInfo, buffer, allocation, allocInfo);
		AURORA_VK_CHECK(result, VK_SUCCESS, "Failed to create buffer!");
		return true;
	}

	// == Buffer Barriers ==
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

	// ========== Image Barriers ==========

	VkImageMemoryBarrier2 EmitLayoutTransitionBarrier(VkImage image,
												VkImageLayout oldLayout,
												VkImageLayout newLayout,
												VkImageSubresourceRange range,
												VkPipelineStageFlags2 srcStageMask,
												VkAccessFlags2 srcAccessMask,
												VkPipelineStageFlags2 dstStageMask,
												VkAccessFlags2 dstAccessMask)
	{
		VkImageMemoryBarrier2 barrier{};
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
		VkImageMemoryBarrier2 barrier{};
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
		VkImageMemoryBarrier2 barrier{};
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