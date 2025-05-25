#pragma once
#include "Core/Core.h"
#include "Core/Logging.h"

#include <vulkan/vulkan.h>

#ifdef AURORA_ASSERT_ENABLED
#define AURA_VK_CHECK_RESULT(x, y, ...) { if(!(CheckVkResult(x, y)) { AURORA_ERROR("Unexpected VkResult (Expected{0}, got {1}: Message: {2}", x, y, __VA_ARGS__); __debugbreak(); } }
#else
#define AURA_VK_CHECK_RESULT(x, y, ...) { x } //we need to pass the function without log message when not in debug mode!
#endif

namespace Aurora { namespace VK {

	static constexpr bool CheckVkResult(VkResult result, VkResult expected)
	{
		return result == expected;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
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
}
