#pragma once

// ============================================================================
//  Utility placement rule - first match wins:
//    1. takes a VkCommandBuffer?              -> VulkanCommands
//    2. creates/destroys a Vk/VMA object?     -> VulkanCreators
//    3. interrogates a VkPhysicalDevice
//       or VkSurfaceKHR?                      -> VulkanQueries
//    4. otherwise, pure function of enums
//       and PODs                              -> VulkanConvert
//       ...returning a string for logging?    -> VulkanToString  (this file)
//
//  This file: diagnostics only. Everything here exists to be read by a human
//  in a log line. Nothing in the engine may branch on these strings.
//
//  If a bucket passes ~300 lines, split it by resource (Image/Buffer),
//  never by adding a table of contents.
// ============================================================================

#include "Aurora/Renderer/Vulkan/VulkanCore.h"

#include <string>

namespace Aurora::VK {

	inline constexpr std::string DeviceTypeToString(VkPhysicalDeviceType type)
	{
		switch (type)
		{
		case VK_PHYSICAL_DEVICE_TYPE_CPU: return "CPU";
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: return "Discrete GPU";
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return "Integrated GPU";
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: return "Virtual GPU";
		default: return "Other";
		}
	}

	inline constexpr std::string ApiVersionToString(uint32_t version)
	{
		uint32_t major = VK_API_VERSION_MAJOR(version);
		uint32_t minor = VK_API_VERSION_MINOR(version);
		uint32_t variant = VK_API_VERSION_VARIANT(version);
		return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(variant);
	}

	inline constexpr std::string LayoutToString(VkImageLayout layout)
	{
		switch (layout)
		{
		case VK_IMAGE_LAYOUT_UNDEFINED: return "Undefined";
		case VK_IMAGE_LAYOUT_GENERAL: return "General";
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: return "Color Attachment Optimal";
		case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL: return "Depth Attachment Optimal";
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: return "Depth Stencil Attachment Optimal";
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL: return "Depth Stencil Read Only Optimal";
		case VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL: return "Read Only Optimal";
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: return "Shader Read Only Optimal";
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: return "Transfer Src Optimal";
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: return "Transfer Dst Optimal";
		case VK_IMAGE_LAYOUT_PREINITIALIZED: return "Preinitialized";
		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: return "Present Source";
		default: return "Unknown Layout";
		}
	}

	inline constexpr std::string FormatToString(VkFormat format)
	{
		switch (format)
		{
			case VK_FORMAT_UNDEFINED: return "Undefined";
			case VK_FORMAT_R8G8B8A8_SRGB: return "R8G8B8A8_SRGB";
			case VK_FORMAT_R8G8B8A8_UNORM: return "R8G8B8A8_UNORM";
			case VK_FORMAT_B8G8R8A8_SRGB: return "B8G8R8A8_SRGB";
			case VK_FORMAT_B8G8R8A8_UNORM: return "B8G8R8A8_UNORM";
			case VK_FORMAT_D32_SFLOAT: return "D32_SFLOAT";
			case VK_FORMAT_D32_SFLOAT_S8_UINT: return "D32_SFLOAT_S8_UINT";
			default: return "Unknown Format";
		}
	}

	inline constexpr std::string ImageUsageFlagsToString(VkImageUsageFlags usage)
	{
		std::string result;
		if ((usage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) == VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
			result += "Transfer Src | ";
		if ((usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT) == VK_IMAGE_USAGE_TRANSFER_DST_BIT)
			result += "Transfer Dst | ";
		if ((usage & VK_IMAGE_USAGE_SAMPLED_BIT) == VK_IMAGE_USAGE_SAMPLED_BIT)
			result += "Sampled | ";
		if ((usage & VK_IMAGE_USAGE_STORAGE_BIT) == VK_IMAGE_USAGE_STORAGE_BIT)
			result += "Storage | ";
		if ((usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) == VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
			result += "Color Attachment | ";
		if ((usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
			result += "Depth Stencil Attachment | ";
		if (result.empty())
			return "None";
		
		return result.substr(0, result.size() - 3); // Remove the trailing " | "
	}

	inline constexpr std::string QueueOwnerToString(QueueOwner owner)
	{
		switch (owner)
		{
			case QueueOwner::UNKNOWN: return "QUEUE_OWNER_UNKNOWN";
			case QueueOwner::GRAPHICS: return "QUEUE_OWNER_GRAPHICS";
			case QueueOwner::PRESENT: return "QUEUE_OWNER_PRESENT";
			case QueueOwner::COMPUTE: return "QUEUE_OWNER_COMPUTE";
			case QueueOwner::TRANSFER: return "QUEUE_OWNER_TRANSFER";
			default: return "INVALID_QUEUE_OWNER";
		}
	}
}
