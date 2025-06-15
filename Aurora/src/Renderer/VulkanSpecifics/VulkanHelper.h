#pragma once
#include "Renderer/VulkanSpecifics/VulkanCore.h"
#include "Renderer/VulkanSpecifics/QueueFamilies.h"

namespace Aurora::VK::Helper {

	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice phDevice, VkSurfaceKHR surface);





}