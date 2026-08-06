#include "Aurora/Renderer/Vulkan/VulkanRenderGraph.h"
#include "Aurora/Profiling/Profiling.h"
#include "Aurora/Core/Core.h"
#include "Aurora/Renderer/Vulkan/Utility/VulkanCommands.h"

#include "Aurora/Aurora.h"
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
		for (auto& [copyName, compiledCopy] : m_CompiledCopies)
		{
			for (auto& fifHandle : compiledCopy.CopyTargetsPerFif)
				DestroyImage(fifHandle);
			//Sources come from Attachments->get cleared there
			compiledCopy.CopySourcesPerFif.clear();
			compiledCopy.CopyTargetsPerFif.clear();				
		}		
		m_CompiledCopies.clear();
	}

	void VulkanRenderGraph::AddRenderPass(const Ref<RenderPass>& renderPass)
	{
		PROFILE_FUNCTION;


		//TOOD: extend this by figuring out the slot based off of the attachment chainings
		m_RenderPasses.push_back(renderPass);
	}

	void VulkanRenderGraph::AddAttachmentCopy(std::string_view copyRequestName, const AttachmentCopyRequest& copyInfo)
	{
		PROFILE_FUNCTION;
		std::string copyName = std::string(copyRequestName);
		if (copyName.empty())
		{
			AURORA_ERROR("Empty CopyRequestName not allowed.");
			return;
		}

		auto it = m_CompiledCopies.find(copyName);
		if (it != m_CompiledCopies.end())
		{
			AURORA_ERROR("Copy request {} already in use.", copyName.c_str());
			return;
		}

		
		const std::vector<CompiledAttachment*> foundAttachments = FindAttachmentByNameInPass(copyInfo.PassName, copyInfo.AttachmentName);
		if (foundAttachments.size() == 0)
		{
			AURORA_ERROR("CopyRequest {}: Could not find any attachments in pass {} with name {}.",
						 copyName.c_str(),
						 copyInfo.PassName.c_str(),
						 copyInfo.AttachmentName.c_str());
			return;
		}

		CompiledAttachmentCopy compiledCopy{};
		compiledCopy.CopySourcesPerFif.reserve(foundAttachments.size());
		compiledCopy.CopyTargetsPerFif.reserve(foundAttachments.size());

		for (size_t i = 0; i < foundAttachments.size(); i++)
			compiledCopy.CopySourcesPerFif.push_back(foundAttachments[i]->Handle);

		Ref<VulkanResourceManager> resourceManager = GetResourceManager();
		bool success = true;
		for (size_t i = 0; i < compiledCopy.CopySourcesPerFif.size(); i++)
		{
			const auto& src = compiledCopy.CopySourcesPerFif[i];
			VulkanImageData* srcData = resourceManager->GetImageData(src);
			if (!srcData)
			{
				AURORA_ERROR("Found copy source image handle ({}) has no data.", (uint64_t)src);
				success = false;
				break;
			}
			ImageSpecification copyTargetInfo{};
			copyTargetInfo.Format = static_cast<Format>(srcData->Format);
			copyTargetInfo.Height = srcData->Height;
			copyTargetInfo.Width = srcData->Width;
			copyTargetInfo.MemUsage = MemoryUsage::GPU_ONLY;
			copyTargetInfo.MipLevels = 1;
			std::string name = copyInfo.AttachmentName + "_FiF" + std::to_string(i);
			copyTargetInfo.Name = name;
			copyTargetInfo.Tiling = ImageTiling::OPTIMAL;
			copyTargetInfo.Usage = ImageUsageFlags::SAMPLED | ImageUsageFlags::TRANSFER_DST;
			ImageHandle handle = CreateImage(copyTargetInfo);
			if (handle == ImageHandle::INVALID_HANDLE)
			{
				AURORA_ERROR("Failed to create copy target image ({})", name.c_str());
				success = false;
				break;
			}
			compiledCopy.CopyTargetsPerFif.push_back(handle);
		}

		if (!success)
		{
			AURORA_WARN("Failed to set up compiled copy {}. Clearing up already created resources.", copyName.c_str());
			for (size_t i = 0; i < compiledCopy.CopyTargetsPerFif.size(); i++)
				DestroyImage(compiledCopy.CopyTargetsPerFif[i]);

			compiledCopy.CopySourcesPerFif.clear();
			compiledCopy.CopyTargetsPerFif.clear();
			return;
		}

		for (size_t i = 0; i < foundAttachments.size(); i++)
			foundAttachments[i]->CopyRequested = true;

		compiledCopy.PassName = copyInfo.PassName;
		compiledCopy.AttachmentName = copyInfo.AttachmentName;
		m_CompiledCopies[copyName] = std::move(compiledCopy);
	}

	void VulkanRenderGraph::RemoveAttachmentCopy(std::string_view copyRequestName)
	{
		PROFILE_FUNCTION;


		std::string copyName = std::string(copyRequestName);
		auto it = m_CompiledCopies.find(copyName);
		if (it == m_CompiledCopies.end())
		{
			AURORA_WARN("Copy request {} was not registered.", copyName.c_str());
			return;
		}

		for (auto& target : it->second.CopyTargetsPerFif)
			DestroyImage(target);
		it->second.CopyTargetsPerFif.clear();
		it->second.CopySourcesPerFif.clear();

		const std::vector<CompiledAttachment*> foundAttachments = FindAttachmentByNameInPass(it->second.PassName, it->second.AttachmentName);
		if (foundAttachments.size() == 0)
		{
			AURORA_WARN("No attachments were found during removal of attachment copy request {}. This should never happen and might cause leaks!", copyName.c_str());
		}
		else
			for (size_t i = 0; i < foundAttachments.size(); i++)
				foundAttachments[i]->CopyRequested = false;

		m_CompiledCopies.erase(copyName);		
	}

	ImageHandle VulkanRenderGraph::GetCopyTarget(std::string_view copyRequestName)
	{
		PROFILE_FUNCTION;

		std::string copyName = std::string(copyRequestName);
		auto it = m_CompiledCopies.find(copyName);
		if (it == m_CompiledCopies.end())
		{
			AURORA_WARN("Copy request {} was not registered.", copyName.c_str());
			return ImageHandle::INVALID_HANDLE;
		}

		const uint8_t fif = GetRenderContext()->GetCurrentFrameInFlightIndex();
		uint8_t copyTargetCount = static_cast<uint8_t>(it->second.CopyTargetsPerFif.size());
		if (copyTargetCount <= fif)
		{
			AURORA_WARN("Fif index larger then copy targets array ({})", copyTargetCount);
			return ImageHandle::INVALID_HANDLE;
		}

		return it->second.CopyTargetsPerFif[fif];
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
					CompiledAttachment compiledAttachment{};
					compiledAttachment.Name = colorAttachment.Name;
					compiledAttachment.Handle = colorAttachment.ImageHandlesPerFiF[i];

					VulkanImageData* imageData = resourceManager->GetImageData(compiledAttachment.Handle);
					if (!imageData)
					{
						AURORA_ERROR("Color Attachment found with invalid handle!");
						continue;
					}

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
					slot.DepthAttachment.Name = depthAttachment.Name;
					slot.DepthAttachment.Handle = depthAttachment.ImageHandlesPerFiF[i];
					VulkanImageData* imageData = resourceManager->GetImageData(slot.DepthAttachment.Handle);
					if (!imageData)
					{
						AURORA_ERROR("Depth Attachment found with invalid handle!");
						continue;
					}

					VkRenderingAttachmentInfo attachmentInfo{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
					attachmentInfo.pNext = nullptr;
					attachmentInfo.imageView = imageData->ImageView;
					//CAUTION: Bake here, but compare with actual layout during Execute, injecting layout transition
					//			barrier if needed
					attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
					attachmentInfo.resolveMode = VK_RESOLVE_MODE_NONE;
					attachmentInfo.resolveImageView = VK_NULL_HANDLE;
					attachmentInfo.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
					attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
					attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
					attachmentInfo.clearValue.depthStencil.depth = renderPass->GetClearDepth();
					attachmentInfo.clearValue.depthStencil.stencil = 0;
					slot.DepthAttachment.AttachmentInfo = attachmentInfo;
					pass.HasDepthAttachment = true;
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
		Ref<VulkanRenderer> renderer = GetRenderer();
		Ref<VulkanResourceManager> resManager = GetResourceManager();

		VulkanFrame& frameData = renderContext->GetCurrentFrameData();
		VkCommandBuffer cmd = frameData.CommandBuffer;

		renderer->BindBindlessPipeline(cmd);

		for (const auto& pass : m_CompiledPasses)
		{
			CompiledPassSlot slot = pass.Slots[frameData.FrameIndex];

			std::unordered_map<std::string, VulkanImageData*> copySources;
			std::vector<VkRenderingAttachmentInfo> colorAttachments;
			colorAttachments.reserve(slot.ColorAttachments.size());
			for (uint32_t i = 0; i < slot.ColorAttachments.size(); i++)
			{
				CompiledAttachment attachment = slot.ColorAttachments[i];
				VulkanImageData* imageData = resManager->GetImageData(attachment.Handle);
				if(!CheckAndTransitImage(imageData, cmd, attachment.AttachmentInfo.imageLayout))
					AURORA_ERROR("Image data of attachment '{}' nullptr", attachment.Name.c_str());
				else
				{
					colorAttachments.push_back(attachment.AttachmentInfo);
					if (attachment.CopyRequested)
						copySources[attachment.Name] = imageData;
				}
			}

			VkRenderingInfo renderingInfo{ VK_STRUCTURE_TYPE_RENDERING_INFO };
			renderingInfo.pNext = nullptr;
			renderingInfo.flags = 0;
			renderingInfo.colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size());
			renderingInfo.pColorAttachments = colorAttachments.data();			

			renderingInfo.pStencilAttachment = nullptr;
			renderingInfo.renderArea = pass.RenderArea;
			renderingInfo.layerCount = 1;
			renderingInfo.viewMask = 0;

			if (pass.HasDepthAttachment)
			{
				VulkanImageData* imageData = resManager->GetImageData(slot.DepthAttachment.Handle);
				if (!CheckAndTransitImage(imageData, cmd, slot.DepthAttachment.AttachmentInfo.imageLayout))
					AURORA_ERROR("Image data of attachment 'depth' nullptr");
				else
					renderingInfo.pDepthAttachment = &slot.DepthAttachment.AttachmentInfo;

				if (slot.DepthAttachment.CopyRequested)
					copySources[slot.DepthAttachment.Name] = imageData;
			}
			else
				renderingInfo.pDepthAttachment = nullptr;
			vkCmdBeginRendering(cmd, &renderingInfo);

			VkViewport viewport{};
			viewport.width = static_cast<float>(pass.RenderArea.extent.width);
			viewport.height = static_cast<float>(pass.RenderArea.extent.height);
			viewport.x = static_cast<float>(pass.RenderArea.offset.x);
			viewport.y = static_cast<float>(pass.RenderArea.offset.y);
			vkCmdSetViewport(cmd, 0, 1, &viewport);
			vkCmdSetScissor(cmd, 0, 1, &pass.RenderArea);
			renderer->Draw(cmd, 3);

			vkCmdEndRendering(cmd);		
			
			//Manage copies
			if (copySources.size() == 0)
				continue;
			for (auto& [copyName, copy] : m_CompiledCopies)
			{
				if(std::strcmp(copy.PassName.c_str(), pass.Name.c_str()) != 0)
				   continue;

				VulkanImageData* dst = resManager->GetImageData(copy.CopyTargetsPerFif[frameData.FrameIndex]);
				if (!dst)
				{
					AURORA_ERROR("VulkanImageData of target image in frame {} of attachment {} is nullptr", frameData.FrameIndex, copy.AttachmentName);
					continue;
				}
				CheckAndTransitImage(dst, cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
				VulkanImageData* src = copySources.at(copy.AttachmentName);
				CheckAndTransitImage(src, cmd, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
				Commands::BlitImageToImage(cmd, src->Image, src->Width, src->Height, dst->Image, src->Width, src->Height);
				CheckAndTransitImage(dst, cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}
		}
	}

	bool VulkanRenderGraph::CheckAndTransitImage(VulkanImageData* imageData, VkCommandBuffer cmd, VkImageLayout targetLayout)
	{
		PROFILE_FUNCTION;

		if (!imageData)		
			return false;		

		if (imageData->Layout != targetLayout)
		{
			imageData->Layout = Commands::TransitionImageLayout(cmd,
										  imageData->Image,
										  imageData->Format,
										  imageData->Layout,
										  targetLayout);
		}
		return true;
	}


	const std::vector<CompiledAttachment*> VulkanRenderGraph::FindAttachmentByNameInPass(std::string_view passName, std::string_view attachmentName)
	{
		PROFILE_FUNCTION;

		std::string passNameStr = std::string(passName);
		std::string attachmentNameStr = std::string(attachmentName);

		CompiledPass* foundPass = nullptr;
		std::vector<CompiledAttachment*> foundAttachments;
		for (size_t i = 0; i < m_CompiledPasses.size(); i++)
		{
			if (std::strcmp(m_CompiledPasses[i].Name.c_str(), passNameStr.c_str()) != 0)
				continue;
			foundPass = &m_CompiledPasses[i];
		}
		if (!foundPass)
		{
			AURORA_ERROR("PassName {} not found.", passNameStr.c_str());
			return foundAttachments;
		}


		int foundColorIndex = -1;
		bool isDepth = false;
		for (size_t fif = 0; fif < foundPass->Slots.size(); fif++)
		{
			auto& slot = foundPass->Slots[fif];
			if (foundColorIndex > -1)
			{
				foundAttachments.push_back(&slot.ColorAttachments[foundColorIndex]);
				continue;
			}

			if (isDepth)
			{
				foundAttachments.push_back(&slot.DepthAttachment);
				continue;
			}

			if (std::strcmp(slot.DepthAttachment.Name.c_str(), attachmentNameStr.c_str()) == 0)
			{
				isDepth = true;
				foundAttachments.push_back(&slot.DepthAttachment);
				continue;
			}

			for (size_t i = 0; i < slot.ColorAttachments.size(); i++)
			{
				auto& colorAttachment = slot.ColorAttachments[i];
				if (std::strcmp(colorAttachment.Name.c_str(), attachmentNameStr.c_str()) != 0)
					continue;

				foundColorIndex = static_cast<int>(i);
				foundAttachments.push_back(&colorAttachment);
				break;
			}
		}

		return foundAttachments;
	}


}