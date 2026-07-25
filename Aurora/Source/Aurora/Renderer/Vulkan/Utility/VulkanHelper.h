#pragma once
#include "Aurora/Renderer/Vulkan/VulkanCore.h"

namespace Aurora::VK::Helper {


	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice phDevice, VkSurfaceKHR surface);

	const SwapchainSupportDetails GetSwapSupportDetails(VkPhysicalDevice phDevice, VkSurfaceKHR surface);
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats, VkFormat preferredFormat, VkColorSpaceKHR preferredColorSpace);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availableModes, VkPresentModeKHR preferred);

	/// <summary>
	/// Method to query the swap extent. If currentExtent is set to UINT32_MAX, use framebufferWidth/Height clamped to min/max image extent.
	/// </summary>
	/// <param name="capabilities"></param>
	/// <param name="framebufferWidth">In pixels e.g. from glfwGetFramebufferSize</param>
	/// <param name="framebufferHeight">In pixels e.g. from glfwGetFramebufferSize</param>
	/// <returns></returns>
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t framebufferWidth, uint32_t framebufferHeight);

	// ========== Images ==========
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

	VkImageAspectFlags GetAspectFlagsFromFormat(VkFormat format);

	// ========== Buffers ==========
	void CopyBufferToBuffer(
		VkDevice device,
		const void* data,
		VkBuffer dstBuffer,
		VmaAllocation dstAllocation,
		size_t dstOffset,
		size_t size
	);
}