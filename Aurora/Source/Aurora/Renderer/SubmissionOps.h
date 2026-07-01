#pragma once

#include "Aurora/Renderer/Handles.h"
#include "Aurora/Renderer/Vulkan/VulkanCore.h"

namespace Aurora::VK {

	enum class SubmissionOpType : uint8_t
	{
		COPY_BUFFER = 0,
	};

	struct SubmissionOp
	{
		SubmissionOpType Type;
	};
}
