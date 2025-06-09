#pragma once
#include "Core/Core.h"
#include "Core/Logging.h"

#include <vulkan/vulkan.h>

#ifdef AURORA_DEBUG_MODE
#define AURORA_VK_VALIDATION 1
#else
#define AURORA_VK_VALIDATION 0
#endif


#ifdef AURORA_ASSERT_ENABLED && AURORA_VK_VALIDATION
#define AURORA_VK_CHECK(x, y, ...) { if(!CheckVkResult(x, y)) { AURORA_ERROR("Unexpected VkResult. Message: {}",  __VA_ARGS__); __debugbreak(); } }
#else
#define AURORA_VK_CHECK(x, y, ...) x  //we need to pass the function without log message when not in debug mode!
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

	inline static bool s_EnabledDebugUtils = false;
	static constexpr void SetVkObjDebugName(VkDevice device, VkObjectType objType, uint64_t objHandle, const char* name)
	{
		if (!s_EnabledDebugUtils)
			return;

		VkDebugUtilsObjectNameInfoEXT nameInfo{ VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
		nameInfo.pNext = nullptr;
		nameInfo.objectType = objType;
		nameInfo.objectHandle = objHandle;
		nameInfo.pObjectName = name;

		AURORA_VK_CHECK(SetDebugUtilsObjectNameEXT(device, &nameInfo), VK_SUCCESS, "Failed to create debug name info!");
	}

	static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			s_EnabledDebugUtils = true;
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else
			return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
			s_EnabledDebugUtils = false;
		}
	}

	static VkResult SetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo)
	{
		auto func = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT");
		if (func != nullptr)		
			return func(device, pNameInfo);
		return VK_INCOMPLETE;
	}
}
}
