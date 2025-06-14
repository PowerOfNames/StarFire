#pragma once


namespace Aurora { namespace VK {

	struct PhysicalDeviceLimits
	{
		struct QueueFamilyLimits
		{
			bool HasDedicatedComputeQueue;
			bool HasDedicatedTransferQueue;
		} QueueFamLimits;


	};


} }
