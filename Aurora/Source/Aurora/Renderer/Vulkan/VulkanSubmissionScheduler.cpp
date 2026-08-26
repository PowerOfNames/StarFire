#include "Aurora/Renderer/Vulkan/VulkanSubmissionScheduler.h"
#include "Aurora/Core/Core.h"
#include "Aurora/Renderer/Vulkan/VulkanCore.h"
#include "AuroraInternal.h"
#include "Aurora/Renderer/Vulkan/VulkanResourceManager.h"
#include "Aurora/Renderer/Vulkan/Utility/VulkanConvert.h"

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


		Ref<VulkanResourceManager> res = GetResourceManager();
		VulkanBufferData* srcData = res->GetBufferData(src);
		if (!srcData)
		{
			AURORA_ERROR("Invalid src buffer handle!");
			return *this;
		}
		VulkanBufferData* dstData = res->GetBufferData(dst);
		if (!dstData)
		{
			AURORA_ERROR("Invalid dst buffer handle");
			return *this;
		}

		if (dstData->Size < srcData->Size)
		{
			AURORA_ERROR("Dst data too small for src");
			return *this;
		}

		VulkanBufferCopyOp op;
		op.Type = SubmissionOpType::COPY_BUFFER;
		op.SrcBuffer = srcData->Buffer;
		op.DstBuffer = dstData->Buffer;
		op.Size = srcData->Size;
		op.SrcOffset = srcData->Offset;
		op.DstOffset = dstData->Offset;

		op.SrcUsage = srcData->Usage;
		op.DstUsage = dstData->Usage;

		op.SrcHandle = src;
		op.DstHandle = dst;

		op.SrcCurrentOwner = srcData->CurrentOwner;
		op.DstCurrentOwner = dstData->CurrentOwner;
		op.DstNextOwner = nextOwner;

		op.DestroySrc = destroySrc;

		m_DeferredCopyBufferToBufferSubmissions.push_back(std::move(op));

		return *this;
	}

	void VulkanSubmissionScheduler::ScheduleSubmissions(bool forceNow)
	{
		PROFILE_FUNCTION;

		m_Context->AddDeferredBufferCopySubmissionOps(m_DeferredCopyBufferToBufferSubmissions, forceNow);
	}
}