#pragma once
#include "Aurora/Renderer/Handles.h"
#include "Aurora/Renderer/Image.h"

#include "Substrate/RefCounted.h"
#include "Substrate/RefPtr.h"

#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace Aurora {

	struct ColorAttachmentSpecification
	{
		std::string Name = "Color attachment name";
		ImageSpecification ImageSpecs{};
		glm::vec4 ClearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	};

	struct DepthAttachmentSpecification
	{
		std::string Name = "Depth attachment name";
		ImageSpecification ImageSpecs{};
		float ClearDepth = 1.0f;
	};

	struct RenderPassSpecification
	{
		std::string Name = "Render pass name";
		std::vector<ColorAttachmentSpecification> ColorAttachments;
		DepthAttachmentSpecification DepthAttachment{};
		glm::uvec2 RenderArea;
	};

	struct RenderPassAttachment
	{
		std::string Name;
		std::vector<ImageHandle> ImageHandlesPerFiF;
		glm::vec4 ClearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		uint32_t Index = 0;
	};

	class RenderPass : public Substrate::RefCounted
	{
	public:
		virtual ~RenderPass() = default;
		virtual void Destroy() = 0;

		virtual const RenderPassAttachment& GetColorAttachment(std::string_view attachmentName) const = 0;
		virtual const std::vector<RenderPassAttachment>& GetColorAttachments() const = 0;
		virtual const RenderPassAttachment& GetDepthAttachment() const = 0;
		virtual bool HasDepthAttachment() const = 0;

		virtual const glm::uvec2& GetRenderArea() const = 0;
		virtual const float GetClearDepth() const = 0;

		static Ref<RenderPass> Create(const RenderPassSpecification& specs);
		virtual const RenderPassSpecification& GetSpecification() const = 0;
	};
}
