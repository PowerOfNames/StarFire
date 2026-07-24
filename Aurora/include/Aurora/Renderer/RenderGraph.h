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

		virtual void AddRenderPass(const Ref<RenderPass>& renderPass) = 0;
		virtual void AddAttachmentCopy(std::string_view copyRequestName, const AttachmentCopyRequest& copyInfo) = 0;
		virtual void RemoveAttachmentCopy(std::string_view copyRequestName) = 0;

		virtual ImageHandle GetCopyTarget(std::string_view copyRequestName) = 0;

		virtual void Compile() = 0;

		// This will take in the scene data in a data driven way (either already culled, or the render graph will do the culling itself)
		virtual void Execute(const VertexBufferHandle vbHandle, const IndexBufferHandle ibHandle) = 0;

		virtual const RenderGraphSpecification& GetSpecification() const = 0;
	};

}
