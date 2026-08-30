#pragma once
#include "Aurora/Renderer/RenderPass.h"
#include "Aurora/Renderer/Handles.h"

#include <unordered_map>

namespace Aurora::VK {


	class VulkanRenderPass : public RenderPass
	{
	public:
		VulkanRenderPass(const RenderPassSpecification& specs);
		virtual ~VulkanRenderPass() = default;
		void Compile() override;
		void Destroy() override;

		void OnResize(uint32_t width, uint32_t height) override;

		const RenderPassAttachment& GetColorAttachment(std::string_view attachmentName) const override;
		const std::vector<RenderPassAttachment>& GetColorAttachments() const override {	return m_ColorAttachments; }
		const RenderPassAttachment& GetDepthAttachment() const override;
		inline bool HasDepthAttachment() const override { return m_HasDepthAttachment; }

		const glm::uvec2& GetRenderArea() const override { return m_Specification.RenderArea; }
		const float GetClearDepth() const override {return m_Specification.DepthAttachment.ClearDepth; }


		inline const RenderPassSpecification& GetSpecification() const override { return m_Specification; }

	private:
		ImageHandle CreateAttachment(const ImageSpecification& attachmentSpecs);
		void ClearAttachments();
	private:
		RenderPassSpecification m_Specification;
		bool m_Compiled = false;

		std::unordered_map<std::string, uint32_t> m_ColorAttachmentIndices;
		std::vector<RenderPassAttachment> m_ColorAttachments;
		RenderPassAttachment m_DepthAttachment;
		bool m_HasDepthAttachment = false;
	};

}
