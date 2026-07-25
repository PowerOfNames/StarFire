#pragma once

// ============================================================================
//  Utility placement rule - first match wins:
//    1. takes a VkCommandBuffer?              -> VulkanCommands
//    2. creates/destroys a Vk/VMA object?     -> VulkanCreators
//    3. interrogates a VkPhysicalDevice
//       or VkSurfaceKHR?                      -> VulkanQueries
//    4. otherwise, pure function of enums
//       and PODs                              -> VulkanConvert   (this file)
//       ...returning a string for logging?    -> VulkanToString
//
//  This file: pure derivations. No device, no allocator, no command buffer,
//  no state. Everything here is a total function of its arguments, which is
//  what makes it the only bucket that is unit-testable without a GPU.
//
//  If a bucket passes ~300 lines, split it by resource (Image/Buffer),
//  never by adding a table of contents.
// ============================================================================

#include "Aurora/Core/Logging.h"
#include "Aurora/Renderer/Types.h"
#include "Aurora/Renderer/Vulkan/VulkanCore.h"
#include "Aurora/Renderer/Vulkan/Utility/VulkanToString.h"

namespace Aurora::VK::Convert {

	// ========== Usage -> pipeline stage / access ==========

	inline constexpr VkPipelineStageFlags2 GetStageFromBufferUsage(BufferUsageFlags usage, QueueOwner queue = QueueOwner::UNKNOWN)
	{
		if (queue == QueueOwner::TRANSFER)
			return VK_PIPELINE_STAGE_2_TRANSFER_BIT;

		if (queue == QueueOwner::COMPUTE)
			return VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;

		VkPipelineStageFlags2 stageFlags = 0;
		if ((usage & BufferUsageFlags::VERTEX_BUFFER) == BufferUsageFlags::VERTEX_BUFFER)
			stageFlags |= VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT;
		if ((usage & BufferUsageFlags::INDEX_BUFFER) == BufferUsageFlags::INDEX_BUFFER)
			stageFlags |= VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT;
		if ((usage & BufferUsageFlags::UNIFORM_BUFFER) == BufferUsageFlags::UNIFORM_BUFFER)
			stageFlags |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
		if ((usage & BufferUsageFlags::STORAGE_BUFFER) == BufferUsageFlags::STORAGE_BUFFER)
			stageFlags |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
		if ((usage & BufferUsageFlags::INDIRECT_BUFFER) == BufferUsageFlags::INDIRECT_BUFFER)
			stageFlags |= VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
		if ((usage & BufferUsageFlags::TRANSFER_SRC) == BufferUsageFlags::TRANSFER_SRC || (usage & BufferUsageFlags::TRANSFER_DST) == BufferUsageFlags::TRANSFER_DST)
			stageFlags |= VK_PIPELINE_STAGE_2_TRANSFER_BIT;
		return stageFlags;
	}

	inline constexpr VkAccessFlags2 GetAccessFromBufferUsage(BufferUsageFlags usage)
	{
		VkAccessFlags2 accessFlags = 0;
		if ((usage & BufferUsageFlags::VERTEX_BUFFER) == BufferUsageFlags::VERTEX_BUFFER)
			accessFlags |= VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;
		if ((usage & BufferUsageFlags::INDEX_BUFFER) == BufferUsageFlags::INDEX_BUFFER)
			accessFlags |= VK_ACCESS_2_INDEX_READ_BIT;
		if ((usage & BufferUsageFlags::UNIFORM_BUFFER) == BufferUsageFlags::UNIFORM_BUFFER)
			accessFlags |= VK_ACCESS_2_UNIFORM_READ_BIT;
		if ((usage & BufferUsageFlags::STORAGE_BUFFER) == BufferUsageFlags::STORAGE_BUFFER)
			accessFlags |= VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
		if ((usage & BufferUsageFlags::INDIRECT_BUFFER) == BufferUsageFlags::INDIRECT_BUFFER)
			accessFlags |= VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
		if ((usage & BufferUsageFlags::TRANSFER_SRC) == BufferUsageFlags::TRANSFER_SRC)
			accessFlags |= VK_ACCESS_2_TRANSFER_READ_BIT;
		if ((usage & BufferUsageFlags::TRANSFER_DST) == BufferUsageFlags::TRANSFER_DST)
			accessFlags |= VK_ACCESS_2_TRANSFER_WRITE_BIT;
		return accessFlags;
	}

	inline constexpr VkPipelineStageFlags2 GetStageFromImageUsage(ImageUsageFlags usage, QueueOwner queue = QueueOwner::UNKNOWN)
	{
		if (queue == QueueOwner::TRANSFER)
			return VK_PIPELINE_STAGE_2_TRANSFER_BIT;

		if (queue == QueueOwner::COMPUTE)
			return VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;

		VkPipelineStageFlags2 stageFlags = 0;
		if ((usage & ImageUsageFlags::SAMPLED) == ImageUsageFlags::SAMPLED)
			stageFlags |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
		if ((usage & ImageUsageFlags::STORAGE) == ImageUsageFlags::STORAGE)
			stageFlags |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
		if ((usage & ImageUsageFlags::COLOR_ATTACHMENT) == ImageUsageFlags::COLOR_ATTACHMENT)
			stageFlags |= VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
		if ((usage & ImageUsageFlags::DEPTH_STENCIL_ATTACHMENT) == ImageUsageFlags::DEPTH_STENCIL_ATTACHMENT)
			stageFlags |= VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
		if ((usage & ImageUsageFlags::TRANSFER_SRC) == ImageUsageFlags::TRANSFER_SRC || (usage & ImageUsageFlags::TRANSFER_DST) == ImageUsageFlags::TRANSFER_DST)
			stageFlags |= VK_PIPELINE_STAGE_2_TRANSFER_BIT;
		return stageFlags;
	}

	inline constexpr VkAccessFlags2 GetAccessFromImageUsage(ImageUsageFlags usage)
	{
		VkAccessFlags2 accessFlags = 0;
		if ((usage & ImageUsageFlags::SAMPLED) == ImageUsageFlags::SAMPLED)
			accessFlags |= VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
		if ((usage & ImageUsageFlags::STORAGE) == ImageUsageFlags::STORAGE)
			accessFlags |= VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
		if ((usage & ImageUsageFlags::COLOR_ATTACHMENT) == ImageUsageFlags::COLOR_ATTACHMENT)
			accessFlags |= VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
		if ((usage & ImageUsageFlags::DEPTH_STENCIL_ATTACHMENT) == ImageUsageFlags::DEPTH_STENCIL_ATTACHMENT)
			accessFlags |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		if ((usage & ImageUsageFlags::TRANSFER_SRC) == ImageUsageFlags::TRANSFER_SRC || (usage & ImageUsageFlags::TRANSFER_DST) == ImageUsageFlags::TRANSFER_DST)
			accessFlags |= VK_ACCESS_2_TRANSFER_READ_BIT | VK_ACCESS_2_TRANSFER_WRITE_BIT;
		return accessFlags;
	}

	// ========== Format -> aspect ==========

	// Not constexpr: warns on unknown formats. Everything else here is.
	inline VkImageAspectFlags GetAspectFlagsFromFormat(VkFormat format)
	{
		VkImageAspectFlags aspects = 0;
		switch (format)
		{
			// Color formats
			case VK_FORMAT_R8G8B8A8_SRGB:
			case VK_FORMAT_R8G8B8A8_UNORM:
			case VK_FORMAT_B8G8R8A8_SRGB:
			case VK_FORMAT_B8G8R8A8_UNORM: aspects |= VK_IMAGE_ASPECT_COLOR_BIT; break;
			// Depth formats
			case VK_FORMAT_D32_SFLOAT: aspects |= VK_IMAGE_ASPECT_DEPTH_BIT; break;
			// Depth + Stencil formats
			case VK_FORMAT_D32_SFLOAT_S8_UINT: aspects |= VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT; break;
			default:
				AURORA_WARN("Unhandles VK_FORMAT {} detected. Falling back to VK_IMAGE_ASPECT_COLOR_BIT", FormatToString(format).c_str());
				return VK_IMAGE_ASPECT_COLOR_BIT;
		}
		return aspects;
	}
}
