#pragma once
#include "Aurora/Core/Core.h"
#include "Aurora/Renderer/Vulkan/VulkanContext.h"
#include "Aurora/Renderer/Vulkan/VulkanResourceManager.h"

namespace Aurora {
	// ========== General ==========
	Ref<VK::VulkanContext> GetRenderContext();
	Ref<VK::VulkanResourceManager> GetResourceManager();
}
