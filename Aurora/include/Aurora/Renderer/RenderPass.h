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

	// Lifecycle contract — the RenderGraph drives the pass.
	//
	// A RenderPass holds a specification and owns its attachment images. It cannot render on its own:
	// recording and execution live entirely in the RenderGraph. So the graph is the sole driver:
	//
	//   1. A pass belongs to at exactly one graph. After RenderGraph::AddRenderPass, the graph owns its
	//      compilation, its extent and its teardown.
	//   2. Do not call Compile(), OnResize() or Destroy() on a pass a graph holds — call the graph's
	//      equivalent instead. The graph caches image views and the render area when it compiles;
	//      resizing a pass behind its back leaves those pointing at destroyed objects. Debug builds warn.
	//   3. Add every pass before RenderGraph::Compile().
	//   4. Release your Ref after RenderGraph::Destroy(). Destroy() clears the attachment containers, so
	//      a surviving Ref can only be used to read out of bounds.
	//
	// Create() allocates no GPU memory; Compile() does. A pass that is created and never added to a graph
	// therefore owns nothing and needs no teardown.
	class RenderPass : public Substrate::RefCounted
	{
	public:
		virtual ~RenderPass() = default;
		virtual void Compile() = 0;
		virtual void Destroy() = 0;


		virtual void OnResize(uint32_t width, uint32_t height) = 0;

		virtual void SetOwnedByRenderGraph(bool owned) = 0;

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
