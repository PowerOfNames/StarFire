#include "Aurora/Renderer/Vulkan/Utility/VulkanCreators.h"
#include "Aurora/Core/Logging.h"

#include "Aurora/Profiling/Profiling.h"
#include "Aurora/Renderer/Vulkan/Utility/VulkanConvert.h"

namespace Aurora::VK::Creators {

	// ========== Images ==========
	bool CreateImage(VmaAllocator allocator, VulkanImageData& data, VmaMemoryUsage memUsage)
	{
		PROFILE_FUNCTION;

		if (data.Width == 0 || data.Height == 0)
		{
			AURORA_WARN("Unable to create an image with extent ({}; {})", data.Width, data.Height);
			return false;
		}

		if (data.MipLevels == 0)
		{
			AURORA_WARN("Unable to create an image with 0 mip levels");
			return false;
		}

		VkImageCreateInfo imageCreateInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		imageCreateInfo.pNext = nullptr;
		imageCreateInfo.flags = 0;
		imageCreateInfo.format = data.Format;
		imageCreateInfo.extent = { data.Width, data.Height, 1 };
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.mipLevels = data.MipLevels;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.tiling = data.Tiling;
		imageCreateInfo.usage = Convert::ToVkImageUsageFlags(data.Usage);
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.arrayLayers = 1;

		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = memUsage;
		VkResult result = vmaCreateImage(allocator, &imageCreateInfo, &allocCreateInfo, &data.Image, &data.Allocation, nullptr);
		AURORA_VK_CHECK(result, VK_SUCCESS, "Failed to create image!");

		return true;
	}

	bool CreateImageView(VkDevice device, const VkAllocationCallbacks* allocationCbs, VulkanImageData& data)
	{
		PROFILE_FUNCTION;


		VkImageViewCreateInfo viewCreateInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		viewCreateInfo.pNext = nullptr;
		viewCreateInfo.flags = 0;
		viewCreateInfo.image = data.Image;
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCreateInfo.format = data.Format;
		viewCreateInfo.components = {
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY
		};
		viewCreateInfo.subresourceRange.aspectMask = Convert::GetAspectFlagsFromFormat(data.Format);
		viewCreateInfo.subresourceRange.baseArrayLayer = 0;
		viewCreateInfo.subresourceRange.layerCount = 1;
		viewCreateInfo.subresourceRange.baseMipLevel = 0;
		viewCreateInfo.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;

		AURORA_VK_CHECK(vkCreateImageView(device, &viewCreateInfo, allocationCbs, &data.ImageView), VK_SUCCESS, "Failed to create image view!");
		return true;
	}

	// ========== Buffers ==========
	bool CreateBuffer(VmaAllocator allocator, VulkanBufferData& data, VmaMemoryUsage memUsage)
	{
		PROFILE_FUNCTION;


		if (data.Size == 0)
		{
			AURORA_WARN("Unable to create a buffer with size 0");
			return false;
		}

		VkBufferCreateInfo bufferCreateInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferCreateInfo.pNext = nullptr;
		bufferCreateInfo.flags = 0;
		bufferCreateInfo.size = data.Size;
		bufferCreateInfo.usage = Convert::ToVkBufferUsageFlags(data.Usage);
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = memUsage;
		VkResult result = vmaCreateBuffer(allocator, &bufferCreateInfo, &allocCreateInfo, &data.Buffer, &data.Allocation, &data.AllocationInfo);
		AURORA_VK_CHECK(result, VK_SUCCESS, "Failed to create buffer!");
		return true;
	}
}
