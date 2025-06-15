#pragma once


namespace Aurora::VK {

	struct PhysicalDeviceLimits
	{
		struct QueueFamilyLimits
		{
			bool HasDedicatedComputeQueue;
			bool HasDedicatedTransferQueue;
		} QueueFamLimits;


	};

}
