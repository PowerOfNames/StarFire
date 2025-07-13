#pragma once
#include "Aurora/Renderer/VulkanCore.h"

#include <vector>

namespace Aurora::VK {

	struct SwapchainSupportDetails
	{
		VkSurfaceCapabilitiesKHR Capabilities;
		std::vector<VkSurfaceFormatKHR> Formats;
		std::vector<VkPresentModeKHR> PresentModes;
	};
}