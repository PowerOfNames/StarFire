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

		uint8_t framesInFLight = GetRenderContext()->GetFramesInFlightCount();

		for (const auto& colorAttachmentSpec : m_Specification.ColorAttachments)
		{
			RenderPassAttachment attachment{};
			attachment.Name = colorAttachmentSpec.Name;
			attachment.Index = static_cast<uint32_t>(m_ColorAttachments.size());
			m_ColorAttachmentIndices[colorAttachmentSpec.Name] = static_cast<uint32_t>(m_ColorAttachments.size());
			for (uint8_t i = 0; i < framesInFLight; i++)
			{
				attachment.ImageHandlesPerFiF.push_back(CreateAttachment(colorAttachmentSpec.ImageSpecs));
			}
			attachment.ClearColor = colorAttachmentSpec.ClearColor;
			m_ColorAttachments.push_back(std::move(attachment));
		}

		if (m_Specification.DepthAttachment.ImageSpecs.Format != Format::UNKNOWN)
		{
			m_DepthAttachment.Name = m_Specification.DepthAttachment.Name;
			m_DepthAttachment.Index = 0; // Depth attachment is always at slot 0
			for (uint8_t i = 0; i < framesInFLight; i++)
			{
				m_DepthAttachment.ImageHandlesPerFiF.push_back(CreateAttachment(m_Specification.DepthAttachment.ImageSpecs));
			}
			m_HasDepthAttachment = true;
		}
	}

	void VulkanRenderPass::Destroy()
	{
		PROFILE_FUNCTION;

		AURORA_INFO("Destroying renderPass '{}'...", m_Specification.Name.c_str());
		for (auto& colorAttachment : m_ColorAttachments)
		{
			for (const auto& handle : colorAttachment.ImageHandlesPerFiF)
			{
				DestroyImage(handle);
			}
			colorAttachment.ImageHandlesPerFiF.clear();
		}
		m_ColorAttachmentIndices.clear();
		m_ColorAttachments.clear();
		if (m_HasDepthAttachment)
		{
			for (const auto& handle : m_DepthAttachment.ImageHandlesPerFiF)
			{
				DestroyImage(handle);
			}
		}
		m_DepthAttachment.ImageHandlesPerFiF.clear();
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

	const RenderPassAttachment& VulkanRenderPass::GetColorAttachment(std::string_view attachmentName) const
	{
		PROFILE_FUNCTION;

		auto it = m_ColorAttachmentIndices.find(std::string(attachmentName));
		if (it == m_ColorAttachmentIndices.end())
		{
			AURORA_ERROR("Attachment '{}' not found in render pass '{}'", attachmentName, m_Specification.Name);
			static RenderPassAttachment emptyAttachment;
			return emptyAttachment;
		}
		return m_ColorAttachments[it->second];
	}

	const RenderPassAttachment& VulkanRenderPass::GetDepthAttachment() const
	{
		PROFILE_FUNCTION;

		if (!m_HasDepthAttachment)
		{
			AURORA_ERROR("Render pass '{}' does not have a depth attachment", m_Specification.Name);
			static RenderPassAttachment emptyAttachment;
			return emptyAttachment;
		}
		return m_DepthAttachment;
	}
}