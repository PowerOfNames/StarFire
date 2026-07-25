#pragma once

#include "Aurora/Renderer/Vulkan/VulkanCore.h"
#include "Aurora/Renderer/Types.h"

namespace Aurora::VK {

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
}
