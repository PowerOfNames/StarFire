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
#include "Aurora/Renderer/Image.h"
#include "Aurora/Renderer/Buffer.h"
#include "Aurora/Renderer/Types.h"
#include "Aurora/Renderer/Vulkan/VulkanCore.h"
#include "Aurora/Renderer/Vulkan/Utility/VulkanToString.h"

namespace Aurora::VK::Convert {

	// ========== Aurora -> Vulkan types ==========
	//
	// The Aurora enums in Renderer/Types.h are deliberately assigned Vulkan's
	// numeric values so that conversion is a plain cast. Nothing at the
	// definition site says so, which makes the contract easy to break by adding
	// an enumerator or by Vulkan renumbering one. It is pinned here instead: a
	// mismatch becomes a compile error rather than a wrong image format at
	// runtime. Convert through these rather than casting at the call site.

	static_assert(static_cast<int>(Format::UNKNOWN) == VK_FORMAT_UNDEFINED);
	static_assert(static_cast<int>(Format::RGBA8_UNORM) == VK_FORMAT_R8G8B8A8_UNORM);
	static_assert(static_cast<int>(Format::RGBA8_SRGB) == VK_FORMAT_R8G8B8A8_SRGB);
	static_assert(static_cast<int>(Format::DEPTH32_SFLOAT) == VK_FORMAT_D32_SFLOAT);
	static_assert(static_cast<int>(Format::DEPTH24_STENCIL8) == VK_FORMAT_D24_UNORM_S8_UINT);

	static_assert(static_cast<int>(ImageTiling::OPTIMAL) == VK_IMAGE_TILING_OPTIMAL);
	static_assert(static_cast<int>(ImageTiling::LINEAR) == VK_IMAGE_TILING_LINEAR);

	static_assert(static_cast<int>(MemoryUsage::UNKNOWN) == VMA_MEMORY_USAGE_UNKNOWN);
	static_assert(static_cast<int>(MemoryUsage::GPU_ONLY) == VMA_MEMORY_USAGE_GPU_ONLY);
	static_assert(static_cast<int>(MemoryUsage::CPU_ONLY) == VMA_MEMORY_USAGE_CPU_ONLY);
	static_assert(static_cast<int>(MemoryUsage::CPU_TO_GPU) == VMA_MEMORY_USAGE_CPU_TO_GPU);
	static_assert(static_cast<int>(MemoryUsage::GPU_TO_CPU) == VMA_MEMORY_USAGE_GPU_TO_CPU);

	static_assert(static_cast<int>(ImageUsageFlags::TRANSFER_SRC) == VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
	static_assert(static_cast<int>(ImageUsageFlags::TRANSFER_DST) == VK_IMAGE_USAGE_TRANSFER_DST_BIT);
	static_assert(static_cast<int>(ImageUsageFlags::SAMPLED) == VK_IMAGE_USAGE_SAMPLED_BIT);
	static_assert(static_cast<int>(ImageUsageFlags::STORAGE) == VK_IMAGE_USAGE_STORAGE_BIT);
	static_assert(static_cast<int>(ImageUsageFlags::COLOR_ATTACHMENT) == VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
	static_assert(static_cast<int>(ImageUsageFlags::DEPTH_STENCIL_ATTACHMENT) == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

	static_assert(static_cast<int>(BufferUsageFlags::TRANSFER_SRC) == VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	static_assert(static_cast<int>(BufferUsageFlags::TRANSFER_DST) == VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	static_assert(static_cast<int>(BufferUsageFlags::UNIFORM_BUFFER) == VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	static_assert(static_cast<int>(BufferUsageFlags::STORAGE_BUFFER) == VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	static_assert(static_cast<int>(BufferUsageFlags::INDEX_BUFFER) == VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	static_assert(static_cast<int>(BufferUsageFlags::VERTEX_BUFFER) == VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	static_assert(static_cast<int>(BufferUsageFlags::INDIRECT_BUFFER) == VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);
	static_assert(static_cast<int>(BufferUsageFlags::SHADER_DEVICE_ADDRESS) == VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);

	[[nodiscard]] inline constexpr VkFormat ToVkFormat(Format format)
	{
		return static_cast<VkFormat>(format);
	}

	[[nodiscard]] inline constexpr VkImageTiling ToVkImageTiling(ImageTiling tiling)
	{
		return static_cast<VkImageTiling>(tiling);
	}

	[[nodiscard]] inline constexpr VmaMemoryUsage ToVmaMemoryUsage(MemoryUsage usage)
	{
		return static_cast<VmaMemoryUsage>(usage);
	}

	[[nodiscard]] inline constexpr VkImageUsageFlags ToVkImageUsageFlags(ImageUsageFlags usage)
	{
		return static_cast<VkImageUsageFlags>(usage);
	}

	[[nodiscard]] inline constexpr VkBufferUsageFlags ToVkBufferUsageFlags(BufferUsageFlags usage)
	{
		return static_cast<VkBufferUsageFlags>(usage);
	}

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

	// ========== Specification -> runtime data ==========

	/// <summary>
	/// Translates the public ImageSpecification into the backend's runtime
	/// struct. Creates nothing - the Vulkan object fields (Image, ImageView,
	/// Allocation) are left null for a Creators:: call to fill in.
	/// </summary>
	[[nodiscard]] inline VulkanImageData MakeImageData(const ImageSpecification& spec)
	{
		VulkanImageData data{};
		data.Width = spec.Width;
		data.Height = spec.Height;
		data.MipLevels = spec.MipLevels;
		data.Format = ToVkFormat(spec.Format);
		data.Tiling = ToVkImageTiling(spec.Tiling);
		data.Layout = VK_IMAGE_LAYOUT_UNDEFINED; // Default layout, can be transitioned later
		// TRANSFER_SRC is force-added to every image as a placeholder until the
		// ImageUsage composite enum lands. Carried over verbatim from
		// VulkanResourceManager::CreateImage.
		data.Usage = spec.Usage | ImageUsageFlags::TRANSFER_SRC;
		return data;
	}

	/// <summary>
	/// Translates the public BufferSpecification into the backend's runtime
	/// struct. Creates nothing - the Vulkan object fields (Buffer, Allocation)
	/// are left null for a Creators:: call to fill in.
	/// </summary>
	[[nodiscard]] inline VulkanBufferData MakeBufferData(const BufferSpecification& spec)
	{
		VulkanBufferData data{};
		data.Size = spec.Size;
		data.Usage = spec.Usage;
		return data;
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
			case VK_FORMAT_D24_UNORM_S8_UINT:
			case VK_FORMAT_D32_SFLOAT_S8_UINT: aspects |= VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT; break;
			default:
				AURORA_WARN("Unhandles VK_FORMAT {} detected. Falling back to VK_IMAGE_ASPECT_COLOR_BIT", FormatToString(format).c_str());
				return VK_IMAGE_ASPECT_COLOR_BIT;
		}
		return aspects;
	}
}
