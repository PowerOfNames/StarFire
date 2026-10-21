#include "Aurora/Renderer/Vulkan/Utility/VulkanMemory.h"

namespace Aurora::VK::Memory
{
	void WriteMapped(VmaAllocator vmaAllocator, VulkanBufferData& buffer, size_t dstOffset, const void* src, size_t size)
	{
		std::byte* globalsDst = static_cast<std::byte*>(buffer.MappedPtr) + dstOffset;
		std::memcpy(globalsDst, src, size);
		vmaFlushAllocation(vmaAllocator, buffer.Allocation, dstOffset, size);
	}
}