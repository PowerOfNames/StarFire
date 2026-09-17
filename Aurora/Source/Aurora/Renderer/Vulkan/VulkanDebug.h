#pragma once
#include "Aurora/Core/Core.h"
#include "Aurora/Core/Logging.h"

#include <vulkan/vulkan.h>

#ifndef AURORA_VK_VALIDATION_ENABLED
#	if defined(AURORA_DEBUG_MODE)
#		define AURORA_VK_VALIDATION_ENABLED 1
#	else
#		define AURORA_VK_VALIDATION_ENABLED 0
#	endif
#endif

#ifndef AURORA_VK_DEBUG_NAME_ENABLED
#	if defined(AURORA_DEBUG_MODE)
#		define AURORA_VK_DEBUG_NAME_ENABLED 1
#	else
#		define AURORA_VK_DEBUG_NAME_ENABLED 0
#	endif	
#endif

#if AURORA_VK_DEBUG_NAME_ENABLED == 1
#	define AURORA_VK_ATTACH_DEBUG_NAME(Device, ObjectType, Handle, DebugName) do{ Aurora::VK::Debug::SetVkObjDebugName(Device, ObjectType, Handle, DebugName); }while(false)
#else
#	define AURORA_VK_ATTACH_DEBUG_NAME(Device, ObjectType, Handle, DebugName) do{}while(false)
#endif

#if AURORA_CHECK_LEVEL >= 3
#	define AURORA_VK_CHECK_BREAK() SUBSTRATE_DEBUG_BREAK()
#else
#	define AURORA_VK_CHECK_BREAK() do{}while(false)
#endif

#if AURORA_CHECK_LEVEL >= 1
#	define AURORA_VK_CHECK(x, y, ...)											\
	do {																		\
		if(!Aurora::VK::Debug::CheckVkResult(x, y)) {							\
			AURORA_ERROR( __VA_ARGS__);											\
			AURORA_VK_CHECK_BREAK();											\
		}																		\
	} while(false)
#else
#	define AURORA_VK_CHECK(x, y, ...)											\
	do {																		\
		(void)(x);																\
		(void)sizeof(y);														\
		(void)sizeof(Substrate::CheckHelpers::Unused(__VA_ARGS__));				\
	} while(false)
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

