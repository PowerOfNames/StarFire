#include "Aurora/Renderer/VulkanDebug.h"

namespace Aurora::VK::Debug {

	static bool s_DebugNameEnabled = false;

	VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
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

	VkResult SetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo)
	{
		auto func = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT");
		if (func != nullptr)
			return func(device, pNameInfo);
		return VK_INCOMPLETE;
	}

	void SetVkObjDebugName(VkDevice device, VkObjectType objType, uint64_t objHandle, const std::string& name)
	{
		SetVkObjDebugName(device, objType, objHandle, name.c_str());
	}
	void SetVkObjDebugName(VkDevice device, VkObjectType objType, uint64_t objHandle, const char* name)
	{
		if (!s_DebugNameEnabled)
			return;

		VkDebugUtilsObjectNameInfoEXT nameInfo{ VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
		nameInfo.pNext = nullptr;
		nameInfo.objectType = objType;
		nameInfo.objectHandle = objHandle;
		nameInfo.pObjectName = name;

		AURORA_VK_CHECK(SetDebugUtilsObjectNameEXT(device, &nameInfo), VK_SUCCESS, "Failed to create debug name info!");
	}

	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		static PFN_vkCreateDebugUtilsMessengerEXT func = nullptr;

		if (func == nullptr)
		{
			func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
			s_DebugNameEnabled = true;
		}
		
		if (func != nullptr)		
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);

		s_DebugNameEnabled = false;
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		if (!s_DebugNameEnabled)
			return;

		static PFN_vkDestroyDebugUtilsMessengerEXT func = nullptr;
		if(func == nullptr)
			func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		
		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);		
			s_DebugNameEnabled = false;
		}
	}

}