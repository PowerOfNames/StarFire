#pragma once
#include "Aurora/Logging.h"

#include <vulkan/vulkan.h>

namespace Aurora {
	
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
		void* userData)
	{
		switch (messageSeverity)
		{
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			{
				AURORA_TRACE("AuroraVK-Debug Callback: '{0}' \n", callbackData->pMessage);
				break;
			}
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			{
				AURORA_INFO("AuroraVK-Debug Callback: '{0}' \n", callbackData->pMessage);
				break;
			}
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			{
				AURORA_WARN("AuroraVK-Debug Callback: '{0}' \n", callbackData->pMessage);
				break;
			}
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			{
				AURORA_ERROR("AuroraVK-Debug Callback: '{0}' \n", callbackData->pMessage);
				break;
			}
		}
		return VK_FALSE;
	}
}
