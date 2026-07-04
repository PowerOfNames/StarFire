#include "Aurora/Renderer/Vulkan/VulkanRenderGraph.h"
#include "Aurora/Profiling/Profiling.h"

#include "AuroraInternal.h"

namespace Aurora::VK {
	VulkanRenderGraph::VulkanRenderGraph(const RenderGraphSpecification& specs)
		: m_Specification(specs)
	{
		PROFILE_FUNCTION;


	}


	void VulkanRenderGraph::Destroy()
	{
		PROFILE_FUNCTION;


		AURORA_INFO("Destroying renderGraph '{}'...", m_Specification.Name.c_str());


		for (const auto& rp : m_RenderPasses)	
			rp->Destroy();		
		m_RenderPasses.clear();
	}

	void VulkanRenderGraph::AddRenderPass(const Ref<RenderPass>& renderPass)
	{
		PROFILE_FUNCTION;


		//TOOD: extend this by adding an index (position of this renderPass in Graph
		m_RenderPasses.push_back(renderPass);
	}

	void VulkanRenderGraph::AddImageCopy(const ImageCopyInfo& cpyInfo)
	{
		PROFILE_FUNCTION;



	}

	void VulkanRenderGraph::Compile()
	{
		PROFILE_FUNCTION;



	}

	void VulkanRenderGraph::Execute(const VertexBufferHandle vbHandle, const IndexBufferHandle ibHandle)
	{
		// 1. bind rendergraph specific descriptors (probably camera, lights, the main descriptor set basically in bindless rendering)

		// 2. submit render passes in the order they were added (for now, we can just do this, but later we might want to do some more complex scheduling and stuff)
		// This will also be the place to do any culling if we want the render graph to handle it. We can probably also do some more complex stuff here like automatic batching of draw calls, etc.

		Ref<VulkanContext> renderContext = GetRenderContext();
		VulkanFrame& frameData = renderContext->GetCurrentFrameData();
		VkCommandBuffer cmd = frameData.CommandBuffer;


		for (const Ref<RenderPass>& renderPass : m_RenderPasses)
		{
			VkRenderingAttachmentInfo attachmentInfo{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
			attachmentInfo.pNext = nullptr;
		}
	}

}