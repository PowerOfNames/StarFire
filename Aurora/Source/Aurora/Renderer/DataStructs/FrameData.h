#pragma once
#include "Aurora/Renderer/VulkanCore.h"
#include "Aurora/Renderer/DeletionQueue.h"


namespace Aurora::VK {

	struct FrameData
	{
		// We create one pool per frame, such that we later can record command buffers for multiple frames in parallel if needed. We can optimize this later if needed (e.g. one pool per thread, or one pool for transient buffers and one for long-lived buffers)
		VkCommandPool CommandPool = VK_NULL_HANDLE;
		VkCommandBuffer CommandBuffer = VK_NULL_HANDLE;

		// Target
		VkImageView TargetView = VK_NULL_HANDLE; // The image view of the swapchain image that is the target of rendering for this frame. Updated every frame in RenderContext::BeginFrame() after acquiring the next swapchain image.

		//Syncing:
		VkSemaphore ImageAvailableSemaphore = VK_NULL_HANDLE;
		VkSemaphore RenderFinishedSemaphore = VK_NULL_HANDLE;
		VkFence InFlightFence = VK_NULL_HANDLE;

		// Metadata
		VkExtent2D Extent{};
		uint8_t FrameIndex = UINT8_MAX;
		bool InPresentation = false; // Set to true when the frame is being presented (after command buffer submission and before presentation). Used to prevent resource deletion while a frame is still being presented.


		// Per-frame cleanup
		DeletionQueue DeletionQueue{};
	};
}