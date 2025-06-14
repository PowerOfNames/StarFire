#pragma once

#include <optional>

namespace Aurora { namespace VK {

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

		size_t UniqueFamilyIndices()
		{
			size_t count = 0;
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
} }
