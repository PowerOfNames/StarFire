#pragma once
#include "Core/Core.h"
#include "Core/Logging.h"

#include <vulkan/vulkan.h>

#if defined(AURORA_DEBUG_MODE)
#define AURORA_VK_VALIDATION_ENABLED 1
#define AURORA_VK_DEBUG_NAME_ENABLED 1
#else
#define AURORA_VK_VALIDATION 0
#define AURORA_VK_DEBUG_NAME_ENABLED 0
#endif

#if defined(AURORA_VK_DEBUG_NAME_ENABLED)
#define AURORA_VK_ATTACH_DEBUG_NAME(Device, ObjectType, Handle, DebugName) Aurora::VK::Debug::SetVkObjDebugName(Device, ObjectType, Handle, DebugName)
#else
#define AURORA_VK_ATTACH_DEBUG_NAME(Device, ObjectType, Handle, DebugName) do{}while(0)
#endif

#if defined(AURORA_ASSERT_ENABLED) && AURORA_VK_VALIDATION_ENABLED
#define AURORA_VK_CHECK(x, y, ...) { if(!Aurora::VK::Debug::CheckVkResult(x, y)) { AURORA_ERROR("Unexpected VkResult. Message: {}",  __VA_ARGS__); __debugbreak(); } }
#else
#define AURORA_VK_CHECK(x, y, ...) x;  //we need to pass the function without log message when not in debug mode!
#endif

namespace Aurora::VK::Debug {

	// ========== Helper ==========
	inline constexpr bool CheckVkResult(VkResult result, VkResult expected)
	{
		return result == expected;
	}
	void SetVkObjDebugName(VkDevice device, VkObjectType objType, uint64_t objHandle, const std::string& name);
	void SetVkObjDebugName(VkDevice device, VkObjectType objType, uint64_t objHandle, const char* name);

	// ========== Callback ==========
	VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
		void* userData);

	// ========== Function Pointers to EXT ==========
	VkResult SetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo);
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, 
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
		const VkAllocationCallbacks* pAllocator, 
		VkDebugUtilsMessengerEXT* pDebugMessenger);
	void DestroyDebugUtilsMessengerEXT(VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator);
	

}

