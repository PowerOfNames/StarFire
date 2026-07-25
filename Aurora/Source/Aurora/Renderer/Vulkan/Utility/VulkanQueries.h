#pragma once

// ============================================================================
//  Utility placement rule - first match wins:
//    1. takes a VkCommandBuffer?              -> VulkanCommands
//    2. creates/destroys a Vk/VMA object?     -> VulkanCreators
//    3. interrogates a VkPhysicalDevice
//       or VkSurfaceKHR?                      -> VulkanQueries   (this file)
//    4. otherwise, pure function of enums
//       and PODs                              -> VulkanConvert
//       ...returning a string for logging?    -> VulkanToString
//
//  This file: interrogating the physical device and surface, and selecting
//  from what they report. Startup-time work - these run while building the
//  device and the swapchain, not per frame.
//
//  If a bucket passes ~300 lines, split it by resource (Image/Buffer),
//  never by adding a table of contents.
// ============================================================================

#include "Aurora/Renderer/Vulkan/VulkanCore.h"

#include <vector>

namespace Aurora::VK::Queries {

	// ========== Device ==========
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice phDevice, VkSurfaceKHR surface);

	// ========== Swapchain ==========
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
