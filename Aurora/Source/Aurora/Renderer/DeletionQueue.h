#pragma once

#include "Aurora/Renderer/Vulkan/VulkanDebug.h"
#include "Aurora/Renderer/Vulkan/VMA.h"

#include <vulkan/vulkan.h>

#include <deque>
#include <functional>
#include <vector>


namespace Aurora::VK {

	using DeletionFunction = std::function<void(VkDevice, VmaAllocator, const VkAllocationCallbacks*)>;

	struct ResourceSubmissionWaits
	{
		uint64_t Graphics = 0;
		uint64_t Compute = 0;
		uint64_t Transfer = 0;
	};

	struct DeletionEntry
	{
		DeletionFunction Deleter;
		ResourceSubmissionWaits WaitValue{};
		bool RequiresWait = false;
	};

	struct DeletionQueue
	{
		// storing whole functions is not optimal, but it is the easiest way to ensure that all necessary data for deletion is captured and that deletions are executed in order. We can optimize this later if needed.
		// (Better store arrays of VkHandles per type (buckets) and destroy them in order)
		std::deque<DeletionEntry> DeletionEntries;
		std::vector<DeletionEntry> UnstampedEntries;

		void SubmitDeletion(DeletionFunction func)
		{
			DeletionEntries.push_back({ func, {}, false });
		}
		
		void SubmitWaitingDeletion(DeletionFunction func)
		{
			UnstampedEntries.push_back({ func, {}, true });
		}

		/// <summary>
		/// We basically stamp the wait values of the current frame to all unstamped entries. This is done when we know the current wait values of the semaphores, which is after we submit the command buffers for the current frame. 
		/// This way we can ensure that resources are not deleted until they are no longer in use by the GPU.
		/// </summary>
		/// <param name="waitValues"></param>
		void StampWaitValues(const ResourceSubmissionWaits& waitValues)
		{
			//This should work because we dont reiterate and size should not change.
			for (auto& entry : UnstampedEntries)
			{				
				entry.WaitValue = waitValues;
				DeletionEntries.push_back(std::move(entry));
			}
			UnstampedEntries.clear();
			//we keep capacity to reduce reallocations later. Might be worth doing some intrumentations later to figure out how large we should reserve
		}

		void Flush(VkDevice device, 
				   VkSemaphore graphicsSemaphore, 
				   VkSemaphore computeSemaphore, 
				   VkSemaphore transferSemaphore, 
				   VmaAllocator allocator, 
				   const VkAllocationCallbacks* allocCbs)
		{
			std::deque<DeletionEntry> stillPending;

			uint64_t graphicsValue = 0;
			uint64_t computeValue = 0;
			uint64_t transferValue = 0;

			AURORA_VK_CHECK(vkGetSemaphoreCounterValue(device, graphicsSemaphore, &graphicsValue), VK_SUCCESS, "Failed to get graphics semaphore counter value!");
			AURORA_VK_CHECK(vkGetSemaphoreCounterValue(device, computeSemaphore, &computeValue), VK_SUCCESS, "Failed to get compute semaphore counter value!");
			AURORA_VK_CHECK(vkGetSemaphoreCounterValue(device, transferSemaphore, &transferValue), VK_SUCCESS, "Failed to get transfer semaphore counter value!");

			for (auto it = DeletionEntries.rbegin(); it != DeletionEntries.rend(); ++it)
			{

				if (it->RequiresWait)
				{
					if (graphicsValue < it->WaitValue.Graphics)
					{
						stillPending.push_front(std::move(*it));
						continue;
					}
					if (computeValue < it->WaitValue.Compute)
					{
						stillPending.push_front(std::move(*it));
						continue;
					}
					if (transferValue < it->WaitValue.Transfer)
					{
						stillPending.push_front(std::move(*it));
						continue;
					}
				}

				it->Deleter(device, allocator, allocCbs);
			}
			DeletionEntries = std::move(stillPending);
		}
	};

}