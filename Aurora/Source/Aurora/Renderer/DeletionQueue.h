#pragma once

#include <deque>
#include <functional>

namespace Aurora::VK {


	struct DeletionQueue
	{
		// storing whole functions is not optimal, but it is the easiest way to ensure that all necessary data for deletion is captured and that deletions are executed in order. We can optimize this later if needed.
		// (Better store arrays of VkHandles per type (buckets) and destroy them in order)
		std::deque<std::function<void()>> DeletionFunctions;
		void SubmitDeletion(std::function<void()> func)
		{
			DeletionFunctions.push_back(func);
		}

		void FlushDeletions()
		{
			while (!DeletionFunctions.empty())
			{
				std::function<void()> func = DeletionFunctions.front();
				func();
				DeletionFunctions.pop_front();
			}
		}
	};

}