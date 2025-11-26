#pragma once
#include "Aurora/Renderer/VulkanCore.h"
#include "Aurora/Renderer/DataStructs/QueueFamilies.h"
#include "Aurora/Renderer/DataStructs/SwapchainSupportDetails.h"

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
}