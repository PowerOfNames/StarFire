#pragma once

// ============================================================================
//  Utility placement rule - first match wins:
//    1. takes a VkCommandBuffer?              -> VulkanCommands
//    2. creates/destroys a Vk/VMA object?     -> VulkanCreators
//    3. interrogates a VkPhysicalDevice
//       or VkSurfaceKHR?                      -> VulkanQueries
//    3b. reads or writes the contents of an
//        existing allocation (map, write,
//        flush, invalidate)?                  -> VulkanMemory    (this file)
//    4. otherwise, pure function of enums
//       and PODs                              -> VulkanConvert
//       ...returning a string for logging?    -> VulkanToString
//
//  This file: host access to the bytes of an allocation that already exists.
//  The allocation is created and destroyed in Creators; a GPU-side copy is
//  recorded in Commands. What lives here touches memory through the CPU
//  mapping and keeps it coherent (flush after write, invalidate before read).
//
//  No bounds checks - callers validate before calling in.
//
//  If a bucket passes ~300 lines, split it by resource (Image/Buffer),
//  never by adding a table of contents.
// ============================================================================

#include "Aurora/Renderer/Vulkan/VulkanCore.h"

namespace Aurora::VK::Memory {

	void WriteMapped(VmaAllocator vmaAllocator, VulkanBufferData& buffer, size_t dstOffset, const void* src, size_t size);


	template<typename T>
	void WriteMapped(VmaAllocator vmaAllocator, VulkanBufferData& buffer, size_t dstOffset, const T& value)
	{
		WriteMapped(vmaAllocator, buffer, dstOffset, &value, sizeof(T));
	}
}
