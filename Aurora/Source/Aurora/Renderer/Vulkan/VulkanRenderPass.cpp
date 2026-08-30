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
	}

	void VulkanRenderPass::Destroy()
	{
		PROFILE_FUNCTION;

		AURORA_INFO("Destroying renderPass '{}'...", m_Specification.Name.c_str());
		ClearAttachments();
		m_Compiled = false;
		m_HasDepthAttachment = false;
	}

	void VulkanRenderPass::ClearAttachments()
	{
		PROFILE_FUNCTION;

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
				DestroyImage(handle);
			m_DepthAttachment.ImageHandlesPerFiF.clear();
			m_HasDepthAttachment = false;
		}
	}

	void VulkanRenderPass::Compile()
	{
		PROFILE_FUNCTION;
		// Nothing to do here for Vulkan, as we are using dynamic rendering

		if(m_Compiled)
			ClearAttachments();

		uint8_t framesInFLight = GetRenderContext()->GetFramesInFlightCount();

		for (const auto& colorAttachmentSpec : m_Specification.ColorAttachments)
		{
			RenderPassAttachment attachment{};
			attachment.Name = colorAttachmentSpec.Name;
			attachment.Index = static_cast<uint32_t>(m_ColorAttachments.size());
			attachment.ClearColor = colorAttachmentSpec.ClearColor;			
			for (uint8_t i = 0; i < framesInFLight; i++)			
				attachment.ImageHandlesPerFiF.push_back(CreateAttachment(colorAttachmentSpec.ImageSpecs));
			
			m_ColorAttachmentIndices[colorAttachmentSpec.Name] = static_cast<uint32_t>(m_ColorAttachments.size());
			m_ColorAttachments.push_back(std::move(attachment));
		}

		if (m_Specification.DepthAttachment.ImageSpecs.Format != Format::UNKNOWN)
		{
			m_DepthAttachment.Name = m_Specification.DepthAttachment.Name;
			m_DepthAttachment.Index = 0; // Depth attachment is always at slot 0
			m_HasDepthAttachment = true;
			for (uint8_t i = 0; i < framesInFLight; i++)		
				m_DepthAttachment.ImageHandlesPerFiF.push_back(CreateAttachment(m_Specification.DepthAttachment.ImageSpecs));		
		}

		m_Compiled = true;
	}

	void VulkanRenderPass::OnResize(uint32_t width, uint32_t height)
	{
		PROFILE_FUNCTION;
		if (width == m_Specification.RenderArea.x && height == m_Specification.RenderArea.y)
			return;

		m_Specification.RenderArea.x = width;
		m_Specification.RenderArea.y = height;
		
		for (auto& colorAttachment : m_Specification.ColorAttachments)
		{
			if (colorAttachment.ImageSpecs.AllowResize)
			{
				colorAttachment.ImageSpecs.Width = width;
				colorAttachment.ImageSpecs.Height = height;
			}
		}
		if (m_Specification.DepthAttachment.ImageSpecs.Format != Format::UNKNOWN && m_Specification.DepthAttachment.ImageSpecs.AllowResize)
		{
			m_Specification.DepthAttachment.ImageSpecs.Width = width;
			m_Specification.DepthAttachment.ImageSpecs.Height = height;
		}		
	}

	ImageHandle VulkanRenderPass::CreateAttachment(const ImageSpecification& attachmentSpecs)
	{
		PROFILE_FUNCTION;
		
		ImageHandle handle = CreateImage(attachmentSpecs);
		if (handle == ImageHandle::INVALID_HANDLE)
		{
			AURORA_ERROR("Failed to create image for render pass attachment {}!", attachmentSpecs.Name);
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