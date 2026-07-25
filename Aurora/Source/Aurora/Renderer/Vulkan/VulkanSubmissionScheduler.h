#pragma once

#include "Aurora/Renderer/Vulkan/VulkanCore.h"
#include "Aurora/Renderer/Vulkan/VulkanContext.h"

#include <vector>

namespace Aurora::VK{
	

	class VulkanSubmissionScheduler
	{

	public:		
		VulkanSubmissionScheduler(const Ref<VulkanContext>& context);
		~VulkanSubmissionScheduler();

		VulkanSubmissionScheduler& CopyBufferToBuffer(BufferHandle src, BufferHandle dst, bool destroySrc = true, QueueOwner nextOwner = QueueOwner::UNKNOWN);

		void ScheduleSubmissions(bool forceNow = false);

	private:		
		Ref<VulkanContext> m_Context;

		std::vector<VulkanBufferCopyOp> m_DeferredCopyBufferToBufferSubmissions;

	};
}
