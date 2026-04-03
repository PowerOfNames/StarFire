#include "Aurora/ImGui/ImGuiRenderer.h"
#include "AuroraInternal.h"

#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>

#include <vulkan/vulkan.h>

namespace Aurora::ImGuiImpl {

	struct ImGuiVulkanData
	{
		VkCommandBuffer CommandBuffer;
		VkPipeline Pipeline;
	};

	void Init()
	{
		Ref<VK::RenderContext> renderContext = GetRenderContext();

		ImGui_ImplVulkan_InitInfo initInfo = {};
		initInfo.Instance = VK_NULL_HANDLE; // TODO: Get Vulkan instance from renderer
		initInfo.PhysicalDevice = VK_NULL_HANDLE; // TODO: Get Vulkan physical device from renderer
		initInfo.Device = VK_NULL_HANDLE; // TODO: Get Vulkan logical device from renderer
		initInfo.QueueFamily = 0; // TODO: Get Vulkan queue family index from renderer
		initInfo.Queue = VK_NULL_HANDLE; // TODO: Get Vulkan queue from renderer
		initInfo.DescriptorPool = VK_NULL_HANDLE; // TODO: Create a Vulkan descriptor pool for ImGui
		initInfo.DescriptorPoolSize = 0; // Optional: set to create internal descriptor pool automatically instead of using DescriptorPool.
		initInfo.MinImageCount = 2; // TODO: Set to the minimum number of images in the swap chain
		initInfo.ImageCount = 2; // TODO: Set to the number of images in the swap chain
		initInfo.PipelineCache = VK_NULL_HANDLE; // Optional: set to a Vulkan pipeline cache if you have one
		initInfo.Allocator = nullptr; // Optional: set to a Vulkan allocation callbacks if you have one
		initInfo.PipelineInfoMain.RenderPass = VK_NULL_HANDLE; // TODO: Set to the Vulkan render pass used for the main viewport
		initInfo.PipelineInfoMain.Subpass = 0; // TODO: Set to the subpass index used for the main viewport
		initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		initInfo.UseDynamicRendering = false; // TODO: Set to true if you want to use dynamic rendering
		initInfo.CheckVkResultFn = nullptr; // Optional: set to a function to check Vulkan results
		ImGui_ImplVulkan_Init(&initInfo);
	}

	void Shutdown()
	{
		ImGui_ImplVulkan_Shutdown();
	}

	void BeginFrame()
	{
		ImGui_ImplVulkan_NewFrame();
	}

	void EndFrame()
	{
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), VK_NULL_HANDLE);
	}
}