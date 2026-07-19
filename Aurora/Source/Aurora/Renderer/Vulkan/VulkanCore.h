#pragma once

#include "Aurora/Renderer/Vulkan/VMA.h"
#include "Aurora/Renderer/Vulkan/VulkanDebug.h"
#include "Aurora/Renderer/DeletionQueue.h"
#include "Aurora/Renderer/Handles.h"
#include "Aurora/Renderer/Types.h"
#include "Aurora/Renderer/SubmissionOps.h"

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

namespace Aurora::VK {

	struct VulkanFrame
	{
		// We create one pool per frame, such that we later can record command buffers for multiple frames in parallel if needed. We can optimize this later if needed (e.g. one pool per thread, or one pool for transient buffers and one for long-lived buffers)
		VkCommandPool CommandPool = VK_NULL_HANDLE;
		VkCommandBuffer CommandBuffer = VK_NULL_HANDLE;

		// Target
		VkImage TargetImage = VK_NULL_HANDLE; // The swapchain image that is the target of rendering for this frame. Updated every frame in RenderContext::BeginFrame() after acquiring the next swapchain image.
		VkImageView TargetView = VK_NULL_HANDLE; // The image view of the swapchain image that is the target of rendering for this frame. Updated every frame in RenderContext::BeginFrame() after acquiring the next swapchain image.
		VkImageLayout TargetLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		//Syncing:
		VkFence InFlightFence = VK_NULL_HANDLE;

		// Metadata
		VkExtent2D Extent{};
		uint8_t FrameIndex = UINT8_MAX;
		bool InPresentation = false; // Set to true when the frame is being presented (after command buffer submission and before presentation). Used to prevent resource deletion while a frame is still being presented.


		// Per-frame cleanup
		DeletionQueue DeletionQueue{};
	};

	struct QueueFamilies
	{
		VkQueue Graphics = VK_NULL_HANDLE;
		VkQueue Present = VK_NULL_HANDLE;
		VkQueue Compute = VK_NULL_HANDLE;

		//Move this out when time into transfer operation handler
		VkQueue Transfer = VK_NULL_HANDLE;
	};

	struct QueueFamilyIndices
	{
		uint32_t Graphics = UINT32_MAX;
		uint32_t Present = UINT32_MAX;
		uint32_t Compute = UINT32_MAX;
		uint32_t Transfer = UINT32_MAX;

		//Preferred if true
		bool SamePresentGraphics = false;
		bool HasDedicatedTransfer = false;
		bool HasDedicatedCompute = false;

		//Mainly for integrated GPUs
		uint32_t Unified = UINT32_MAX;
		uint32_t UnifiedCount = 0;

		bool IsComplete()
		{
			return Present != UINT32_MAX && Graphics != UINT32_MAX && Compute != UINT32_MAX && Transfer != UINT32_MAX;
		}

		uint8_t UniqueFamilyIndices()
		{
			uint8_t count = 0;
			if (SamePresentGraphics)
				count++;
			else
				count += 2;
			if (HasDedicatedTransfer)
				count++;
			if (HasDedicatedCompute)
				count++;
			return count;
		}
	};

	struct TimelineSemaphore
	{
		VkSemaphore Semaphore = VK_NULL_HANDLE;
		uint64_t Value = 0;
	};

	struct PendingResourceUpload
	{
		VkSemaphore SignalSemaphore = VK_NULL_HANDLE;
		uint64_t SignalValue = 0;
		BufferHandle Handle = BufferHandle::INVALID_HANDLE;
	};

	enum class QueueOwner : uint8_t
	{
		UNKNOWN = 0,
		GRAPHICS,
		PRESENT,
		COMPUTE,
		TRANSFER,
		MAX_QUEUE_OWNERS
	};

	inline static constexpr std::string QueueOwnerToString(QueueOwner owner)
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


	struct DeviceRequirements
	{
	};

	struct SwapchainSupportDetails
	{
		VkSurfaceCapabilitiesKHR Capabilities;
		std::vector<VkSurfaceFormatKHR> Formats;
		std::vector<VkPresentModeKHR> PresentModes;
	};

	struct PhysicalDeviceLimits
	{
		struct QueueFamilyLimits
		{
			bool HasDedicatedComputeQueue;
			bool HasDedicatedTransferQueue;
		} QueueFamLimits;

		struct DescriptorLimits
		{
			uint32_t MaxPerStageDescriptorStorageBuffers;
			uint32_t MaxPerStageDescriptorSampledImages;
			uint32_t MaxPerStageDescriptorStorageImages;
			uint32_t MaxPerStageDescriptorInputAttachments;
			uint32_t MaxPerStageSamplers;
			uint32_t MaxPerStageResources;
		} DescriptorLimits;
	};

	struct VulkanBufferCopyOp : public SubmissionOp
	{
		BufferHandle Src;
		BufferHandle Dst;
		bool DestroySrc;

		QueueOwner NextDstOwner;
	};


	struct VulkanImageData
	{
		VkImage Image = VK_NULL_HANDLE;						//8  - must have
		VkImageView ImageView = VK_NULL_HANDLE;				//16 - must have
		VmaAllocation Allocation = VK_NULL_HANDLE;			//24 - must have
		VkFormat Format = VK_FORMAT_UNDEFINED;				//28 - must have
		VkImageLayout Layout = VK_IMAGE_LAYOUT_UNDEFINED;	//32 - could be stored in a per-renderpass data struct, but for simplicity we just store it here for now
		VkImageTiling Tiling = VK_IMAGE_TILING_OPTIMAL;		//36 - might be extractable by context

		uint32_t Width = 0;									//40 - could be stored in a per-renderpass data struct, but for simplicity we just store it here for now -> could be skipped if context knows sizes (renderpass should know sizes of all targets)
		uint32_t Height = 0;								//44 - could be stored in a per-renderpass data struct, but for simplicity we just store it here for now -> could be skipped if context knows sizes
		uint8_t MipLevels = 1;								//45 -> aligned to !!! 48 !!! bytes - should potentially be optional as well, but for simplicity we just store it here for now

		//Used to track ownership transferring
		QueueOwner LastOwner = QueueOwner::UNKNOWN;			//46 - aligned to 48 bytes
		QueueOwner CurrentOwner = QueueOwner::UNKNOWN;		//47 - aligned to 48 bytes
		QueueOwner NextOwner = QueueOwner::UNKNOWN;			//48 - aligned to 48 bytes
	};

	struct VulkanBufferData
	{
		VkBuffer Buffer = VK_NULL_HANDLE;
		VmaAllocation Allocation = VK_NULL_HANDLE;
		VmaAllocationInfo AllocationInfo{}; 
		size_t Size = 0;
		size_t Offset = 0;

		BufferUsageFlags Usage = BufferUsageFlags::NONE;

		//Used to track ownership transferring
		QueueOwner LastOwner = QueueOwner::UNKNOWN;
		QueueOwner CurrentOwner = QueueOwner::UNKNOWN;
		QueueOwner NextOwner = QueueOwner::UNKNOWN;

		bool IsReady = true;
	};
}