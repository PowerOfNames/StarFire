#pragma once
#include "Aurora/Renderer/Vulkan/VulkanCore.h"

namespace Aurora::VK::Memory {

	void WriteMapped(VmaAllocator vmaAllocator, VulkanBufferData& buffer, size_t dstOffset, const void* src, size_t size);


	template<typename T>
	void WriteMapped(VmaAllocator vmaAllocator, VulkanBufferData& buffer, size_t dstOffset, const T& value)
	{
		WriteMapped(vmaAllocator, buffer, dstOffset, &value, sizeof(T));
	}
}
