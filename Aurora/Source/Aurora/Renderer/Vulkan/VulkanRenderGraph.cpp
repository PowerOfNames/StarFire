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
		ClearCompilations();
	}

	void VulkanRenderGraph::ClearCompilations()
	{
		PROFILE_FUNCTION;


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
		m_Compiled = false;
	}

	void VulkanRenderGraph::OnResize(uint32_t width, uint32_t height)
	{
		PROFILE_FUNCTION;
		if (width == m_Width && height == m_Height)
			return;

		m_Width = width;
		m_Height = height;
		for (const auto& rp : m_RenderPasses)
			rp->OnResize(width, height);

		AURORA_INFO("Resized RenderGraph {} to extent ({}|{})", m_Specification.Name, width, height);
		m_NeedsResize = true;
	}

	void VulkanRenderGraph::Compile()
	{
		PROFILE_FUNCTION;

		if (m_Compiled)		
			ClearCompilations();
		

		Ref<VulkanResourceManager> resourceManager = GetResourceManager();
		const uint8_t fif = GetRenderContext()->GetFramesInFlightCount();

		//For each render pass:
		bool success = true;
		for (const Ref<RenderPass>& renderPass : m_RenderPasses)
		{
			renderPass->Compile();				

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
						success = false;
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
						success = false;
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

				pass.FiFSlots.push_back(std::move(slot));
			}
			const glm::uvec2& renderArea = renderPass->GetRenderArea();
			pass.RenderArea.offset = { 0, 0 };
			pass.RenderArea.extent = { renderArea.x, renderArea.y };

			m_CompiledPasses.push_back(std::move(pass));
		};
		if (!success)
		{
			AURORA_ERROR("Failed to compile rendergraph!");
			ClearCompilations();
			return;
		}

		m_Compiled = true;

		// Even though AddCopyRequest must only be called after Compile, during resize copy targets need to be recompiled as well. 
		// Hence CopyRequest compilation needs to happen here for previously added requests
		for (const auto& [copyName, copyRequest] : m_CopyRequests)
			CompileCopyRequest(copyName);
	}

	void VulkanRenderGraph::Execute(const VertexBufferHandle vbHandle, const IndexBufferHandle ibHandle)
	{
		PROFILE_FUNCTION;


		if (m_NeedsResize)
		{
			Compile();
			m_NeedsResize = false;
		}

		if (!m_Compiled)
		{
			AURORA_WARN("RenderGraph {} needs to be properly compiled before Execution!", m_Specification.Name);
			return;
		}
		
#if defined(AURORA_DEBUG_MODE)
		if (!ValidateCompiledGraph())
		{
			AURORA_ERROR("RenderGraph validation failed!");
			return;
		}
#endif

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
			const CompiledPassSlot& slot = pass.FiFSlots[frameData.FrameIndex];

			std::unordered_map<std::string, VulkanImageData*> copySources;
			std::vector<VkRenderingAttachmentInfo> colorAttachments;
			colorAttachments.reserve(slot.ColorAttachments.size());
			for (uint32_t i = 0; i < slot.ColorAttachments.size(); i++)
			{
				const CompiledAttachment& attachment = slot.ColorAttachments[i];
				VulkanImageData* imageData = resManager->GetImageData(attachment.Handle);
				if (!CheckAndTransitImage(imageData, cmd, attachment.AttachmentInfo.imageLayout))
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
				{
					renderingInfo.pDepthAttachment = &slot.DepthAttachment.AttachmentInfo;
					if (slot.DepthAttachment.CopyRequested)
						copySources[slot.DepthAttachment.Name] = imageData;
				}
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
				if (std::strcmp(copy.PassName.c_str(), pass.Name.c_str()) != 0)
					continue;

				VulkanImageData* dst = resManager->GetImageData(copy.CopyTargetsPerFif[frameData.FrameIndex]);
				if (!CheckAndTransitImage(dst, cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL))				
					AURORA_ERROR("Failed to transition {} destination to DST_OPTIMAL for frame {}", copy.AttachmentName, frameData.FrameIndex);
								
				VulkanImageData* src = copySources.at(copy.AttachmentName);
				if(!CheckAndTransitImage(src, cmd, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL))				
					AURORA_ERROR("Failed to transition {} source to SRC_OPTIMAL for frame {}", copy.AttachmentName, frameData.FrameIndex);
				
				Commands::BlitImageToImage(cmd, src->Image, src->Width, src->Height, dst->Image, src->Width, src->Height);
				if(!CheckAndTransitImage(dst, cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))				
					AURORA_ERROR("Failed to transition {} destination to SHADER_READ_ONLY_OPTIMAL for frame {}", copy.AttachmentName, frameData.FrameIndex);				
			}
		}
	}

	void VulkanRenderGraph::AddRenderPass(const Ref<RenderPass>& renderPass)
	{
		PROFILE_FUNCTION;


		//TOOD: extend this by figuring out the slot based off of the attachment chainings
		m_RenderPasses.push_back(renderPass);
		renderPass->SetOwnedByRenderGraph(true);
	}

	void VulkanRenderGraph::AddAttachmentCopy(std::string_view copyRequestName, const AttachmentCopyRequest& copyInfo)
	{
		PROFILE_FUNCTION;

		if (!m_Compiled)
		{
			AURORA_ERROR("RenderGraph must be compiled before adding attachment copy requests.");
			return;
		}

		if (m_CopyRequests.find(std::string(copyRequestName)) != m_CopyRequests.end())
		{
			AURORA_ERROR("Copy request with name '{}' already exists.", copyRequestName.data());
			return;
		}
		std::string copyName = std::string(copyRequestName);
		if (copyName.empty())
		{
			AURORA_ERROR("Empty CopyRequestName not allowed.");
			return;
		}
		m_CopyRequests[std::string(copyRequestName)] = copyInfo;

		//we only want to compile if no resize is pending to not compile twice without using it
		if(!m_NeedsResize)
			CompileCopyRequest(copyRequestName);
	}

	void VulkanRenderGraph::CompileCopyRequest(std::string_view requestName)
	{
		PROFILE_FUNCTION;


		std::string copyName = std::string(requestName);
		auto copyInfoIt = m_CopyRequests.find(copyName);
		if (copyInfoIt == m_CopyRequests.end())
		{
			AURORA_ERROR("Copy request {} was not registered.", copyName.c_str());
			return;
		}
		const auto& copyInfo = copyInfoIt->second;

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
		if (m_CopyRequests.find(copyName) == m_CopyRequests.end())
		{
			AURORA_WARN("Copy request {} was not registered in m_CopyRequests. This should never happen and might cause leaks!", copyName.c_str());
			return;
		}
		m_CopyRequests.erase(copyName);
		
		auto it = m_CompiledCopies.find(copyName);
		if (it == m_CompiledCopies.end())
		{
			AURORA_TRACE("Copy request {} was not registered.", copyName.c_str());
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
		for (size_t fif = 0; fif < foundPass->FiFSlots.size(); fif++)
		{
			auto& slot = foundPass->FiFSlots[fif];
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

#if defined (AURORA_DEBUG_MODE)
	bool VulkanRenderGraph::ValidateCompiledGraph()
	{
		PROFILE_FUNCTION;
		if (!m_Compiled)
		{
			AURORA_ERROR("RenderGraph '{}' is not compiled! Call Compile() before Execute().", m_Specification.Name);
			return false;
		}

		if (m_CompiledPasses.size() != m_RenderPasses.size())
		{
			AURORA_ERROR("RenderGraph '{}' has invalid number of compiled passes! Expected {}, got {}.", m_Specification.Name, m_RenderPasses.size(), m_CompiledPasses.size());
			return false;
		}

		// The compiled state is FiF-major (FiFSlots[fif].ColorAttachments[color]) while the pass stores it
		// attachment-major (GetColorAttachments()[color].ImageHandlesPerFiF[fif]), so the invariant is a
		// transpose:  FiFSlots[fif].ColorAttachments[c].Handle == GetColorAttachments()[c].ImageHandlesPerFiF[fif]
		const uint8_t maxFif = GetRenderContext()->GetFramesInFlightCount();
		Ref<VulkanResourceManager> resourceManager = GetResourceManager();

		for (size_t passIndex = 0; passIndex < m_CompiledPasses.size(); passIndex++)
		{
			const Ref<RenderPass>& renderPass = m_RenderPasses[passIndex];
			const CompiledPass& compiledPass = m_CompiledPasses[passIndex];
			const std::string& passName = renderPass->GetSpecification().Name;

			// One slot per frame in flight
			if (compiledPass.FiFSlots.size() != maxFif)
			{
				AURORA_ERROR("RenderGraph '{}': pass '{}' has {} FiF slots, expected {}.", m_Specification.Name, passName, compiledPass.FiFSlots.size(), maxFif);
				return false;
			}

			if (compiledPass.HasDepthAttachment != renderPass->HasDepthAttachment())
			{
				AURORA_ERROR("RenderGraph '{}': pass '{}' depth attachment presence mismatch! Expected {}, got {}.", m_Specification.Name, passName, renderPass->HasDepthAttachment(), compiledPass.HasDepthAttachment);
				return false;
			}

			const std::vector<RenderPassAttachment>& colorAttachments = renderPass->GetColorAttachments();

			// ===== Color =====
			for (uint8_t fif = 0; fif < maxFif; fif++)
			{
				const CompiledPassSlot& slot = compiledPass.FiFSlots[fif];
				if (slot.ColorAttachments.size() != colorAttachments.size())
				{
					AURORA_ERROR("RenderGraph '{}': pass '{}' fif {} has {} compiled color attachments, expected {}.", m_Specification.Name, passName, fif, slot.ColorAttachments.size(), colorAttachments.size());
					return false;
				}

				for (size_t colorIndex = 0; colorIndex < colorAttachments.size(); colorIndex++)
				{
					const RenderPassAttachment& attachment = colorAttachments[colorIndex];
					const CompiledAttachment& compiledAttachment = slot.ColorAttachments[colorIndex];

					if (attachment.ImageHandlesPerFiF.size() != maxFif)
					{
						AURORA_ERROR("RenderGraph '{}': pass '{}' color attachment '{}' owns {} handles, expected {}.", m_Specification.Name, passName, attachment.Name, attachment.ImageHandlesPerFiF.size(), maxFif);
						return false;
					}

					// Graph and pass disagree -> the compile mapped the wrong handle into this slot
					if (compiledAttachment.Handle != attachment.ImageHandlesPerFiF[fif])
					{
						AURORA_ERROR("RenderGraph '{}': pass '{}' color attachment '{}' fif {} does not match the pass. Was the pass resized outside the graph?", m_Specification.Name, passName, attachment.Name, fif);
						return false;
					}

					// The baked view is what reaches vkCmdBeginRendering, so check it against the live image
					VulkanImageData* imageData = resourceManager->GetImageData(compiledAttachment.Handle);
					if (!imageData)
					{
						AURORA_ERROR("RenderGraph '{}': pass '{}' color attachment '{}' fif {} refers to a destroyed image.", m_Specification.Name, passName, attachment.Name, fif);
						return false;
					}
					if (compiledAttachment.AttachmentInfo.imageView != imageData->ImageView)
					{
						AURORA_ERROR("RenderGraph '{}': pass '{}' color attachment '{}' fif {} has a stale baked image view. Recompile the graph after recreating attachments.", m_Specification.Name, passName, attachment.Name, fif);
						return false;
					}
				}
			}

			// ===== Depth =====
			if (!compiledPass.HasDepthAttachment)
				continue;

			const RenderPassAttachment& depthAttachment = renderPass->GetDepthAttachment();
			if (depthAttachment.ImageHandlesPerFiF.size() != maxFif)
			{
				AURORA_ERROR("RenderGraph '{}': pass '{}' depth attachment owns {} handles, expected {}.", m_Specification.Name, passName, depthAttachment.ImageHandlesPerFiF.size(), maxFif);
				return false;
			}

			for (uint8_t fif = 0; fif < maxFif; fif++)
			{
				const CompiledAttachment& compiledDepth = compiledPass.FiFSlots[fif].DepthAttachment;

				if (compiledDepth.Handle != depthAttachment.ImageHandlesPerFiF[fif])
				{
					AURORA_ERROR("RenderGraph '{}': pass '{}' depth attachment fif {} does not match the pass. Was the pass resized outside the graph?", m_Specification.Name, passName, fif);
					return false;
				}

				VulkanImageData* imageData = resourceManager->GetImageData(compiledDepth.Handle);
				if (!imageData)
				{
					AURORA_ERROR("RenderGraph '{}': pass '{}' depth attachment fif {} refers to a destroyed image.", m_Specification.Name, passName, fif);
					return false;
				}
				if (compiledDepth.AttachmentInfo.imageView != imageData->ImageView)
				{
					AURORA_ERROR("RenderGraph '{}': pass '{}' depth attachment fif {} has a stale baked image view. Recompile the graph after recreating attachments.", m_Specification.Name, passName, fif);
					return false;
				}
			}
		}
		return true;
	}
#endif
}