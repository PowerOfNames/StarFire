#pragma once

#include <deque>
#include <functional>

#include <vulkan/vulkan.h>

namespace Aurora::VK {

	struct DeletionEntry
	{
		std::function<void()> DeletionFunction;
		VkSemaphore WaitSemaphore = VK_NULL_HANDLE;
		uint64_t WaitValue = 0;
	};

	struct DeletionQueue
	{
		// storing whole functions is not optimal, but it is the easiest way to ensure that all necessary data for deletion is captured and that deletions are executed in order. We can optimize this later if needed.
		// (Better store arrays of VkHandles per type (buckets) and destroy them in order)
		std::deque<DeletionEntry> DeletionEntries;
		void SubmitDeletion(std::function<void()> func, VkSemaphore waitSemaphore = VK_NULL_HANDLE, uint64_t waitValue = 0)
		{
			DeletionEntries.push_back({ func, waitSemaphore, waitValue });
		}

		void Flush(VkDevice device)
		{
			std::deque<DeletionEntry> stillPending;

			for (auto it = DeletionEntries.rbegin(); it != DeletionEntries.rend(); ++it)
			{
				if (it->WaitSemaphore != VK_NULL_HANDLE)
				{
					uint64_t currentValue = 0;
					vkGetSemaphoreCounterValue(device, it->WaitSemaphore, &currentValue);
					if (currentValue < it->WaitValue)
					{
						stillPending.push_front(*it);
						continue;
					}					
				}

				it->DeletionFunction();
			}
			DeletionEntries = std::move(stillPending);
		}
	};

}