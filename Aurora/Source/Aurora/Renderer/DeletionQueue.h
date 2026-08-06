#pragma once

#include "Aurora/Renderer/Vulkan/VulkanCore.h"

#include <deque>
#include <functional>


namespace Aurora::VK {

	using DeletionFunction = std::function<void(VkDevice, VmaAllocator, const VkAllocationCallbacks*)>;

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
		
		void SubmitDeletion(DeletionFunction func)
		{
			DeletionEntries.push_back({ func, {}, false });
		}
		
		void SubmitDeletion(DeletionFunction func, ResourceSubmissionWaits waitValues)
		{
			DeletionEntries.push_back({ func, waitValues, true });
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
						stillPending.push_front(*it);
						continue;
					}
					if (computeValue < it->WaitValue.Compute)
					{
						stillPending.push_front(*it);
						continue;
					}
					if (transferValue < it->WaitValue.Transfer)
					{
						stillPending.push_front(*it);
						continue;
					}
				}

				it->Deleter(device, allocator, allocCbs);
			}
			DeletionEntries = std::move(stillPending);
		}
	};

}