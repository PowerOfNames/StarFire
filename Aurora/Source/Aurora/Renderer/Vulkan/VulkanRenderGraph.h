#pragma once
#include "Aurora/Renderer/RenderGraph.h"
#include "Aurora/Renderer/Handles.h"

#include "Substrate/RefPtr.h"

#include "Aurora/Renderer/Vulkan/VulkanCore.h"
#include "Aurora/Renderer/Vulkan/VulkanContext.h"


#include <vector>

namespace Aurora::VK {

	class VulkanRenderGraph : public RenderGraph
	{
	public:
		VulkanRenderGraph(const RenderGraphSpecification& specs);
		virtual ~VulkanRenderGraph() = default;

		virtual void Destroy() override;

		virtual void AddRenderPass(const Ref<RenderPass>& renderPass) override;
		virtual void AddImageCopy(const ImageCopyInfo& cpyInfo) override;
		
		virtual void Compile() override;

		virtual void Execute(const VertexBufferHandle vbHandle, const IndexBufferHandle ibHandle) override;

		virtual inline const RenderGraphSpecification& GetSpecification() const override { return m_Specification; }

	private:
		RenderGraphSpecification m_Specification;

		std::vector<Ref<RenderPass>> m_RenderPasses;
	};
}