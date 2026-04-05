#include "Aurora/ImGui/ImGuiRenderer.h"

#include "AuroraInternal.h"
#include "Aurora/Renderer/DataStructs/QueueFamilies.h"
#include "Aurora/Renderer/RenderContext.h"

#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>

#include <Aurora/Renderer/VulkanCore.h>

#include <GLFW/glfw3.h>

namespace Aurora::ImGuiImpl {

	struct ImGuiVulkanData
	{
		VkCommandBuffer CommandBuffer;
		VkPipeline Pipeline;
		VkDescriptorPool DescriptorPool;
	} g_ImGuiVkData;

	static void CheckVkResult(VkResult err)
	{
		AURORA_VK_CHECK(err, VK_SUCCESS, "Error in ImGui - Vulkan");
	}

	void Init()
	{
		Ref<VK::RenderContext> renderContext = GetRenderContext();
		VkDevice device = renderContext->GetLogicalDevice();

		VK::QueueFamilies queueFamilies = renderContext->GetQueueFamilies();
		VK::QueueFamilyIndices queueuIndices = renderContext->GetQueueFamilyIndices();

		ImGui_ImplVulkan_InitInfo initInfo = {};
		initInfo.ApiVersion = renderContext->GetApplicationInfo().apiVersion;
		initInfo.Instance = renderContext->GetVulkanInstance();
		initInfo.PhysicalDevice = renderContext->GetPhysicalDevice();
		initInfo.Device = device;
		initInfo.QueueFamily = queueuIndices.Graphics;
		initInfo.Queue = queueFamilies.Graphics;

		// ===== Descriptors =====
		{
			VkDescriptorPoolSize poolSizes[] =
			{
				// we need IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE (8) per font atlas plus as many as additional calls done to ImGui_ImplVulkan_AddTexture()
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE + 1000 },
			};
			VkDescriptorPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
			poolInfo.pNext = nullptr;
			poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			poolInfo.maxSets = 0;
			for (VkDescriptorPoolSize& poolSize : poolSizes)
				poolInfo.maxSets += poolSize.descriptorCount;
			poolInfo.poolSizeCount = (uint32_t)IM_COUNTOF(poolSizes);
			poolInfo.pPoolSizes = poolSizes;
			AURORA_VK_CHECK(vkCreateDescriptorPool(device, &poolInfo, nullptr, &g_ImGuiVkData.DescriptorPool), VK_SUCCESS, "Failed to create ImGui Vulkan descriptor pool.");
		}

		initInfo.DescriptorPool = g_ImGuiVkData.DescriptorPool;
		initInfo.DescriptorPoolSize = 0;
		Ref<VK::Swapchain> swapchain = renderContext->GetSwapchain();
		initInfo.MinImageCount = swapchain->GetSupportDetails().Capabilities.minImageCount;
		initInfo.ImageCount = swapchain->GetImageCount();
		initInfo.Allocator = renderContext->GetAllocationCallbacks(); // Optional: set to a Vulkan allocation callbacks if you have one		
		initInfo.UseDynamicRendering = true;

		VkFormat swapchainImageFormat = swapchain->GetImageFormat();
		initInfo.PipelineInfoMain.PipelineRenderingCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
		initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
		initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.pColorAttachmentFormats = &(swapchainImageFormat);

		initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		initInfo.CheckVkResultFn = CheckVkResult;
		ImGui_ImplVulkan_Init(&initInfo);

		renderContext->SetImGuiActivity(true);
	}

	void Shutdown()
	{
		ImGui_ImplVulkan_Shutdown();
		Ref<VK::RenderContext> renderContext = GetRenderContext();

		vkDestroyDescriptorPool(renderContext->GetLogicalDevice(), g_ImGuiVkData.DescriptorPool, renderContext->GetAllocationCallbacks());
		renderContext->SetImGuiActivity(false);
	}

	void BeginFrame()
	{
		ImGui_ImplVulkan_NewFrame();
	}

	void EndFrame()
	{
		const VK::FrameData& frame = GetRenderContext()->GetCurrentFrameData();
		VkCommandBuffer cmd = frame.CommandBuffer;

		VkClearValue* clear = nullptr;

		VkRenderingAttachmentInfo colorAttachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
		colorAttachment.pNext = nullptr;
		colorAttachment.imageView = frame.TargetView;
		colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		if(clear)
			colorAttachment.clearValue = *clear;
		
		//colorAttachment.resolveMode = VK_RESOLVE_MODE_NONE;
		//colorAttachment.resolveImageView = VK_NULL_HANDLE;
		//colorAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VkRenderingInfo renderingInfo{ VK_STRUCTURE_TYPE_RENDERING_INFO };
		renderingInfo.pNext = nullptr;
		renderingInfo.colorAttachmentCount = 1;
		renderingInfo.pColorAttachments = &colorAttachment;
		renderingInfo.pDepthAttachment = nullptr;
		renderingInfo.pStencilAttachment = nullptr;
		renderingInfo.renderArea.extent = frame.Extent;
		renderingInfo.renderArea.offset = { 0,0 };
		renderingInfo.layerCount = 1;
		renderingInfo.viewMask = 0;
		renderingInfo.flags = 0;

		vkCmdBeginRendering(cmd, &renderingInfo);

		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

		vkCmdEndRendering(cmd);
	}
}