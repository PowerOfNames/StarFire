#include "Aurora/Renderer/Vulkan/VulkanRenderGraph.h"
#include "Aurora/Profiling/Profiling.h"
#include "Aurora/Core/Core.h"

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
		m_CompiledPasses.clear();
		m_ImageCopies.clear();
	}

	void VulkanRenderGraph::AddRenderPass(const Ref<RenderPass>& renderPass)
	{
		PROFILE_FUNCTION;


		//TOOD: extend this by figuring out the slot based off of the attachment chainings
		m_RenderPasses.push_back(renderPass);
	}

	void VulkanRenderGraph::AddImageCopy(const ImageCopyInfo& cpyInfo)
	{
		PROFILE_FUNCTION;



	}

	void VulkanRenderGraph::Compile()
	{
		PROFILE_FUNCTION;

		Ref<VulkanResourceManager> resourceManager = GetResourceManager();
		const uint8_t fif = GetRenderContext()->GetFramesInFlightCount();

		//For each render pass:
		for (const Ref<RenderPass>& renderPass : m_RenderPasses)
		{			
			//Create one CompiledPass
			CompiledPass pass{};
			pass.Name = renderPass->GetSpecification().Name;

			//Per CompiledPass, create one CompiledPassSlot per frame in flight
			for (uint8_t i = 0; i < fif; i++)
			{
				CompiledPassSlot slot{};
				for (const auto& colorAttachment : renderPass->GetColorAttachments())
				{
					CompiledColorAttachment compiledAttachment{};
					compiledAttachment.Handle = colorAttachment.ImageHandlesPerFiF[i];

					VulkanImageData* imageData = resourceManager->GetImageData(compiledAttachment.Handle);

					VkRenderingAttachmentInfo attachmentInfo{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
					attachmentInfo.pNext = nullptr;
					attachmentInfo.imageView = imageData->ImageView;
					//CAUTION: Bake here, but compare with actual layout during Execute, injecting layout transition
					//			barrier if needed
					attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; 
					attachmentInfo.resolveMode = VK_RESOLVE_MODE_NONE;
					attachmentInfo.resolveImageView = VK_NULL_HANDLE;
					attachmentInfo.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
					attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
					attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
					attachmentInfo.clearValue.color = { colorAttachment.ClearColor.r, colorAttachment.ClearColor.g, colorAttachment.ClearColor.b, colorAttachment.ClearColor.a };
					compiledAttachment.AttachmentInfo = attachmentInfo;
					slot.ColorAttachments.push_back(std::move(compiledAttachment));
				}

				if (renderPass->HasDepthAttachment())
				{				
					const RenderPassAttachment& depthAttachment = renderPass->GetDepthAttachment();
					slot.DepthHandle = depthAttachment.ImageHandlesPerFiF[i];
					VulkanImageData* imageData = resourceManager->GetImageData(slot.DepthHandle);
					VkRenderingAttachmentInfo attachmentInfo{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
					attachmentInfo.pNext = nullptr;
					attachmentInfo.imageView = imageData->ImageView;
					//CAUTION: Bake here, but compare with actual layout during Execute, injecting layout transition
					//			barrier if needed
					attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					attachmentInfo.resolveMode = VK_RESOLVE_MODE_NONE;
					attachmentInfo.resolveImageView = VK_NULL_HANDLE;
					attachmentInfo.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
					attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
					attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
					attachmentInfo.clearValue.depthStencil.depth = renderPass->GetClearDepth();
					attachmentInfo.clearValue.depthStencil.stencil = 0;
					slot.DepthAttachmentInfo = attachmentInfo;
				}
				
				pass.Slots.push_back(std::move(slot));
			}
			const glm::uvec2& renderArea = renderPass->GetRenderArea();
			pass.RenderArea.offset = { 0, 0 };
			pass.RenderArea.extent = { renderArea.x, renderArea.y };
			
			m_CompiledPasses.push_back(std::move(pass));
		};		
	}

	void VulkanRenderGraph::Execute(const VertexBufferHandle vbHandle, const IndexBufferHandle ibHandle)
	{
		// 1. bind rendergraph specific descriptors (probably camera, lights, the main descriptor set basically in bindless rendering)

		// 2. submit render passes in the order they were added (for now, we can just do this, but later we might want to do some more complex scheduling and stuff)
		// This will also be the place to do any culling if we want the render graph to handle it. We can probably also do some more complex stuff here like automatic batching of draw calls, etc.

		Ref<VulkanContext> renderContext = GetRenderContext();
		VulkanFrame& frameData = renderContext->GetCurrentFrameData();
		VkCommandBuffer cmd = frameData.CommandBuffer;


		for (const auto& pass : m_CompiledPasses)
		{


		}
	}

}