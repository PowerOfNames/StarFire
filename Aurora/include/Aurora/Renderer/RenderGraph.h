#pragma once
#include "Aurora/Renderer/RenderPass.h"
#include "Aurora/Renderer/Handles.h"

#include "Substrate/RefCounted.h"
#include "Substrate/RefPtr.h"

#include <string>
#include <string_view>
namespace Aurora {

	struct RenderGraphSpecification
	{
		std::string Name = "Default Render Graph";
	};


	struct AttachmentCopyRequest
	{
		std::string PassName;
		std::string AttachmentName;
	};

	class RenderGraph : public ::Substrate::RefCounted
	{
	public:
		virtual ~RenderGraph() = default;
		static Ref<RenderGraph> Create(const RenderGraphSpecification& specs);

		virtual void Destroy() = 0;

		virtual void OnResize(uint32_t width, uint32_t height) = 0;


		// Passes are static topology: add them all before Compile().
		virtual void AddRenderPass(const Ref<RenderPass>& renderPass) = 0;

		// Attachment copies are runtime subscriptions, not part of the compiled topology — register and
		// remove them freely *after* Compile(), which is what makes runtime display-target switching
		// possible (cycling a deferred pass between albedo, normals, depth). Pre-baking a copy for every
		// attachment would allocate images nobody looks at, so a copy only exists while subscribed.
		// Compile() rebuilds every registered copy, so subscriptions survive a resize without re-adding.
		virtual void AddAttachmentCopy(std::string_view copyRequestName, const AttachmentCopyRequest& copyInfo) = 0;
		virtual void RemoveAttachmentCopy(std::string_view copyRequestName) = 0;

		// May return INVALID_HANDLE — the subscription can be registered but not yet compiled (added while
		// a resize is pending), or removed. Check IsHandleValid before use.
		virtual ImageHandle GetCopyTarget(std::string_view copyRequestName) = 0;

		virtual void Compile() = 0;

		// This will take in the scene data in a data driven way (either already culled, or the render graph will do the culling itself)
		virtual void Execute(const VertexBufferHandle vbHandle, const IndexBufferHandle ibHandle) = 0;

		virtual const RenderGraphSpecification& GetSpecification() const = 0;
	};

}
