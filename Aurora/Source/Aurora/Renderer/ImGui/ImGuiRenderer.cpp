#include "Aurora/ImGui/ImGuiRenderer.h"
#include "Aurora/Renderer/Vulkan/ImGuiImpl/VulkanImGuiRenderer.h"

namespace Aurora {
	Ref<ImGuiRenderer> ImGuiRenderer::Create()
	{
		return CreateRef<VK::VulkanImGuiRenderer>();
	}
}