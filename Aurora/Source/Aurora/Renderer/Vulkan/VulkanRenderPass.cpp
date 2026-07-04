#include "Aurora/Renderer/Vulkan/VulkanRenderPass.h"

#include "Aurora/Aurora.h"
#include "AuroraInternal.h"
#include "Aurora/Core/Core.h"
#include "Aurora/Profiling/Profiling.h"
#include "Aurora/Renderer/Types.h"



namespace Aurora::VK {


	VulkanRenderPass::VulkanRenderPass(const RenderPassSpecification& specs)
		: m_Specification(specs)
	{
		PROFILE_FUNCTION;


		for (const auto& colorAttachmentSpec : m_Specification.ColorAttachments)
		{
			m_ColorAttachmentHandles.push_back(CreateAttachment(colorAttachmentSpec));
		}

		if (m_Specification.DepthAttachment.Format != Format::UNKNOWN)
		{
			m_DepthAttachmentHandle = CreateAttachment(m_Specification.DepthAttachment);
		}
	}

	void VulkanRenderPass::Destroy()
	{
		PROFILE_FUNCTION;


		AURORA_INFO("Destroying renderPass '{}'...", m_Specification.Name.c_str());

		if (m_Specification.DepthAttachment.Format != Format::UNKNOWN)
		{
			DestroyImage(m_DepthAttachmentHandle);
		}
		
		for (const auto& colorHandle : m_ColorAttachmentHandles)
		{
			DestroyImage(colorHandle);
		}
	}

	const ImageHandle VulkanRenderPass::GetColorAttachmentHandle(uint32_t index) const
	{
		PROFILE_FUNCTION;


		AURORA_ASSERT(index < m_ColorAttachmentHandles.size(), "Color attachment index out of bounds!");
		return m_ColorAttachmentHandles[index];
	}

	ImageHandle VulkanRenderPass::CreateAttachment(const ImageSpecification& attachmentSpecs)
	{
		PROFILE_FUNCTION;

		
		ImageHandle handle = CreateImage(attachmentSpecs);
		if (handle == ImageHandle::INVALID_HANDLE)
		{
			AURORA_ERROR("Failed to create image for render pass attachment!");
			return ImageHandle::INVALID_HANDLE;
		}
		return handle;
	}
}