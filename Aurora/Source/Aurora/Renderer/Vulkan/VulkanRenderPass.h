#pragma once
#include "Aurora/Renderer/RenderPass.h"
#include "Aurora/Renderer/Handles.h"


namespace Aurora::VK {


	class VulkanRenderPass : public RenderPass
	{
	public:
		VulkanRenderPass(const RenderPassSpecification& specs);
		virtual ~VulkanRenderPass() = default;

		void Destroy() override;

		const ImageHandle GetColorAttachmentHandle(uint32_t index = 0) const override;
		inline const ImageHandle GetDepthAttachmentHandle() const override { return m_DepthAttachmentHandle; }

		inline const RenderPassSpecification& GetSpecification() const override { return m_Specification; }

	private:
		ImageHandle CreateAttachment(const ImageSpecification& attachmentSpecs);

	private:
		RenderPassSpecification m_Specification;

		std::vector<ImageHandle> m_ColorAttachmentHandles;
		ImageHandle m_DepthAttachmentHandle = ImageHandle::INVALID_HANDLE;
	};

}
