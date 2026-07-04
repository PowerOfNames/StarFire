#include "Aurora/Renderer/Vulkan/VulkanSubmissionScheduler.h"
#include "Aurora/Core/Core.h"
#include "Aurora/Renderer/Vulkan/VulkanCore.h"

namespace Aurora::VK {
	VulkanSubmissionScheduler::VulkanSubmissionScheduler(const Ref<VulkanContext>& context)
		: m_Context(context)
	{
	}

	VulkanSubmissionScheduler::~VulkanSubmissionScheduler()
	{
	}


	VulkanSubmissionScheduler& VulkanSubmissionScheduler::CopyBufferToBuffer(BufferHandle src, BufferHandle dst, bool destroySrc /*= true*/, QueueOwner nextOwner /*= QueueOwner::UNKNOWN*/)
	{
		PROFILE_FUNCTION;


		VulkanBufferCopyOp op;
		op.Type = SubmissionOpType::COPY_BUFFER;
		op.Src = src;
		op.Dst = dst;
		op.DestroySrc = destroySrc;
		op.NextDstOwner = nextOwner;

		m_DeferredCopyBufferToBufferSubmissions.push_back(op);

		return *this;
	}

	void VulkanSubmissionScheduler::ScheduleSubmissions(bool forceNow)
	{
		PROFILE_FUNCTION;

		m_Context->AddDeferredBufferCopySubmissionOps(m_DeferredCopyBufferToBufferSubmissions, forceNow);
	}
}