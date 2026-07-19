#include "Aurora/Core/Core.h"
#include "Aurora/Profiling/Profiling.h"
#include "Aurora/Renderer/Vulkan/VulkanSwapchain.h"
#include "Aurora/Renderer/Vulkan/Utility/VulkanHelper.h"
#include "Shaders/ShaderByteCodes.h"

namespace Aurora::VK {

	VulkanSwapchain::VulkanSwapchain(const SwapchainSpecification& spec)
		: m_Specification(spec)
	{
		PROFILE_FUNCTION;

	}

	Ref<VulkanSwapchain> VulkanSwapchain::Create(const SwapchainSpecification& spec)
	{
		return CreateRef<VulkanSwapchain>(spec);
	}

	void VulkanSwapchain::Init()
	{
		PROFILE_FUNCTION;

		if (!CreateSwapchain(m_Specification.InitialExtent.Width, m_Specification.InitialExtent.Height))
		{
			AURORA_TRACE("Failed to create swapchain.");
			return;
		}

		if (!CreateImageViews())
		{
			AURORA_TRACE("Failed to create swapchain image view.");
			return;
		}

		if (!CreateImageSemaphores())
		{
			AURORA_TRACE("Failed to create swapchain image semaphores");
			return;
		}

		if (!CreateRenderPass())
		{
			AURORA_TRACE("Failed to create swapchain render pass handle.");
			return;
		}

		if (!CreateFramebuffers())
		{
			AURORA_TRACE("Failed to create swapchain framebuffers");
			return;
		}

		if (!CreateFallbackPipeline())
		{
			AURORA_TRACE("Failed to create fallabck pipeline");
			//we technically do not need this EO, but will do anyways for development purposes
			return;
		}
	}

	void VulkanSwapchain::CleanupSwapchain()
	{
		PROFILE_FUNCTION;

		AURORA_TRACE("Cleaning up swapchain.");
		vkDeviceWaitIdle(m_Specification.Device);

		for (auto framebuffer : m_Framebuffers)
			vkDestroyFramebuffer(m_Specification.Device, framebuffer, m_Specification.AllocationCallbacks);
		m_Framebuffers.clear();

		for (auto sema : m_ImageAvailableSemaphores)
			vkDestroySemaphore(m_Specification.Device, sema, m_Specification.AllocationCallbacks);
		m_ImageAvailableSemaphores.clear();

		for (auto sema : m_ImageRenderFinishedSemaphores)
			vkDestroySemaphore(m_Specification.Device, sema, m_Specification.AllocationCallbacks);
		m_ImageRenderFinishedSemaphores.clear();

		for (auto imageView : m_ImageViews)
			vkDestroyImageView(m_Specification.Device, imageView, m_Specification.AllocationCallbacks);
		m_ImageViews.clear();

		vkDestroySwapchainKHR(m_Specification.Device, m_Swapchain, m_Specification.AllocationCallbacks);
		m_Swapchain = VK_NULL_HANDLE;
		m_Images.clear();
		AURORA_TRACE("Cleaning swapchin finished.");
	}

	void VulkanSwapchain::Destroy()
	{
		PROFILE_FUNCTION;

		vkDeviceWaitIdle(m_Specification.Device);

		CleanupSwapchain();

		vkDestroyPipeline(m_Specification.Device, m_FallbackPipeline, m_Specification.AllocationCallbacks);
		m_FallbackPipeline = VK_NULL_HANDLE;

		vkDestroyPipelineLayout(m_Specification.Device, m_FallbackPipelineLayout, m_Specification.AllocationCallbacks);
		m_FallbackPipelineLayout = VK_NULL_HANDLE;

		vkDestroyRenderPass(m_Specification.Device, m_RenderPass, m_Specification.AllocationCallbacks);
		m_RenderPass = VK_NULL_HANDLE;


		AURORA_INFO("Destroyed swapchain.");
	}

	bool VulkanSwapchain::PrepareFrame(VulkanFrame& frame)
	{
		PROFILE_FUNCTION;

		// EO, because this only happens if Present captured suboptimal but no resize event was triggered yet
		if (m_NeedsResize)
			return false;

		AURORA_TRACE("Acquire next image {}", frame.FrameIndex);
		vkWaitForFences(m_Specification.Device, 1, &frame.InFlightFence, VK_TRUE, UINT64_MAX);
		vkResetFences(m_Specification.Device, 1, &frame.InFlightFence);

		frame.InPresentation = false;

		VkResult result = vkAcquireNextImageKHR(m_Specification.Device, m_Swapchain, 1'000'000'000 /*1sec*/, m_ImageAvailableSemaphores[frame.FrameIndex], VK_NULL_HANDLE, &m_ImageIndex);
		if (result == VK_SUBOPTIMAL_KHR)
		{
			m_NeedsResize = true;
			AURORA_WARN("Swapchain not optimal.");
			return false;
		}

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			m_NeedsResize = true;
			AURORA_ERROR("Swapchain not usable. Presentation failed and resize required!");
			return false;
		}
		else
		{
			AURORA_ASSERT(result == VK_SUCCESS, "Failed to acquire swap chain image!");
		}
		frame.TargetImage = m_Images[m_ImageIndex];
		frame.TargetView = m_ImageViews[m_ImageIndex];
		frame.Extent = m_Extent;
		frame.TargetLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		vkResetCommandBuffer(frame.CommandBuffer, 0);
		AURORA_TRACE("Acquired image {}", frame.FrameIndex);

		if (m_NeedsResize)
			return false;

		return true;
	}

	bool VulkanSwapchain::SwapImages(VulkanFrame& frame)
	{
		PROFILE_FUNCTION;

		// ===== Submission =====
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.pNext = nullptr;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &frame.CommandBuffer;

		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &m_ImageAvailableSemaphores[frame.FrameIndex]; //wait until image is available to render/draw to
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &m_ImageRenderFinishedSemaphores[m_ImageIndex]; //signal when drawing is finished and ready to be presented
		AURORA_VK_CHECK(vkQueueSubmit(m_Specification.GraphicsQueue, 1, &submitInfo, frame.InFlightFence), VK_SUCCESS, "Failed to submit draw render buffer!");

		frame.InPresentation = true;

		// ===== Presentation =====
		VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		presentInfo.pNext = nullptr;

		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_Swapchain;
		presentInfo.pWaitSemaphores = &m_ImageRenderFinishedSemaphores[m_ImageIndex]; //wait until ready to be presented
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pImageIndices = &m_ImageIndex;
		presentInfo.pResults = nullptr;

		VkResult result;
		result = vkQueuePresentKHR(m_Specification.PresentQueue, &presentInfo);

		//check if the framebuffer resized
		if (result == VK_SUBOPTIMAL_KHR)
		{
			m_NeedsResize = true;
			AURORA_WARN("Swapchain not optimal.");
			return false;
		}
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			m_NeedsResize = true;
			AURORA_ERROR("Swapchain not usable. Presentation failed and resize required!");
			return false;
		}
		AURORA_ASSERT(result == VK_SUCCESS, "Failed to present swap chain image!");

		AURORA_TRACE("Presented frame {}", frame.FrameIndex);
		return true;
	}

	void VulkanSwapchain::OnResize(uint32_t width, uint32_t height)
	{
		PROFILE_FUNCTION;


		if (m_Extent.width == width && m_Extent.height == height)
		{
			m_NeedsResize = false;
			return;
		}

		AURORA_TRACE("Resizing swapchain to [{}|{}]", width, height);
		CleanupSwapchain();

		if (!CreateSwapchain(width, height))
		{
			AURORA_TRACE("Failed to create(resize) swapchain.");
			return;
		}

		if (!CreateImageViews())
		{
			AURORA_TRACE("Failed to create(resize) swapchain image view.");
			return;
		}

		if (!CreateFramebuffers())
		{
			AURORA_TRACE("Failed to create(resize) swapchain framebuffers");
			return;
		}
		AURORA_INFO("Resized swapchain to [{}|{}]", width, height);

		m_NeedsResize = false;
	}

	bool VulkanSwapchain::CreateSwapchain(uint32_t width, uint32_t height)
	{
		PROFILE_FUNCTION;


		SwapchainSupportDetails details = Helper::GetSwapSupportDetails(m_Specification.PhysicalDevice, m_Specification.Surface);

		//VkSurfaceFormatKHR surfaceFormat = Helper::ChooseSwapSurfaceFormat(details.Formats, VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
		VkSurfaceFormatKHR surfaceFormat = Helper::ChooseSwapSurfaceFormat(details.Formats, VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
		VkPresentModeKHR presentMode = Helper::ChooseSwapPresentMode(details.PresentModes, VK_PRESENT_MODE_MAILBOX_KHR);
		VkExtent2D extent = Helper::ChooseSwapExtent(details.Capabilities, width, height);

		uint32_t imageCount = details.Capabilities.minImageCount;
		if (details.Capabilities.maxImageCount > 0 && imageCount > details.Capabilities.maxImageCount)
			imageCount = details.Capabilities.maxImageCount;


		VkSwapchainCreateInfoKHR swapInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
		swapInfo.pNext = nullptr;
		swapInfo.flags = 0;
		swapInfo.surface = m_Specification.Surface;
		swapInfo.minImageCount = imageCount;
		swapInfo.imageFormat = surfaceFormat.format;
		swapInfo.imageColorSpace = surfaceFormat.colorSpace;
		swapInfo.presentMode = presentMode;
		swapInfo.imageExtent = extent;
		swapInfo.imageArrayLayers = 1; // for multi target rendering like VR, other 3D applications
		swapInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		QueueFamilyIndices indices = Helper::FindQueueFamilies(m_Specification.PhysicalDevice, m_Specification.Surface);
		if (indices.SamePresentGraphics)
		{
			swapInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapInfo.queueFamilyIndexCount = 0;
			swapInfo.pQueueFamilyIndices = nullptr;
		}
		else
		{
			uint32_t neededIndices[] = { indices.Graphics, indices.Present };
			swapInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapInfo.queueFamilyIndexCount = 2;
			swapInfo.pQueueFamilyIndices = neededIndices;
		}
		swapInfo.preTransform = details.Capabilities.currentTransform; // no transformation
		swapInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapInfo.clipped = VK_TRUE;
		swapInfo.oldSwapchain = VK_NULL_HANDLE;

		AURORA_VK_CHECK(vkCreateSwapchainKHR(m_Specification.Device, &swapInfo, m_Specification.AllocationCallbacks, &m_Swapchain), VK_SUCCESS, "Failed to create swapchain handle.");
		if (m_Swapchain == VK_NULL_HANDLE)
			return false;
		AURORA_VK_ATTACH_DEBUG_NAME(m_Specification.Device, VK_OBJECT_TYPE_SWAPCHAIN_KHR, (uint64_t)m_Swapchain, "Swapchain");

		m_Extent = extent;
		m_ImageFormat = surfaceFormat.format;

		return true;
	}

	bool VulkanSwapchain::CreateImageViews()
	{
		PROFILE_FUNCTION;

		uint32_t swapImageCount;
		vkGetSwapchainImagesKHR(m_Specification.Device, m_Swapchain, &swapImageCount, nullptr);
		m_Images.resize(swapImageCount);
		vkGetSwapchainImagesKHR(m_Specification.Device, m_Swapchain, &swapImageCount, m_Images.data());


		m_ImageViews.resize(m_Images.size());
		VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		viewInfo.pNext = nullptr;
		viewInfo.flags = 0;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = m_ImageFormat;
		viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.layerCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.levelCount = 1;
		for (size_t i = 0; i < m_Images.size(); i++)
		{
			viewInfo.image = m_Images[i];

			AURORA_VK_CHECK(vkCreateImageView(m_Specification.Device, &viewInfo, nullptr, &m_ImageViews[i]), VK_SUCCESS, "Failed to create swapchain image view.");
			if (m_ImageViews[i] == VK_NULL_HANDLE)
				return false;
			AURORA_VK_ATTACH_DEBUG_NAME(m_Specification.Device, VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)m_ImageViews[i], "Swapchain_ImageView");
		}
		return true;
	}

	bool VulkanSwapchain::CreateImageSemaphores()
	{
		PROFILE_FUNCTION;

		VkSemaphoreCreateInfo semaInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		semaInfo.pNext = nullptr;
		semaInfo.flags = VK_SEMAPHORE_TYPE_BINARY;

		m_ImageAvailableSemaphores.resize(m_Specification.FramesInFlight);
		for (size_t i = 0; i < m_ImageAvailableSemaphores.size(); i++)
		{
			const std::string iString = std::to_string(i);

			AURORA_VK_CHECK(vkCreateSemaphore(m_Specification.Device, &semaInfo, m_Specification.AllocationCallbacks, &m_ImageAvailableSemaphores[i]), VK_SUCCESS, "Failed to create image available semaphore.");
			if (m_ImageAvailableSemaphores[i] == VK_NULL_HANDLE)
				return false;
			const std::string availableName = "Frame_Sema_Ava_" + iString;
			AURORA_VK_ATTACH_DEBUG_NAME(m_Specification.Device, VK_OBJECT_TYPE_SEMAPHORE, (uint64_t)m_ImageAvailableSemaphores[i], availableName.c_str());
		}

		m_ImageRenderFinishedSemaphores.resize(m_Images.size());
		for (size_t i = 0; i < m_ImageRenderFinishedSemaphores.size(); i++)
		{
			const std::string iString = std::to_string(i);

			AURORA_VK_CHECK(vkCreateSemaphore(m_Specification.Device, &semaInfo, m_Specification.AllocationCallbacks, &m_ImageRenderFinishedSemaphores[i]), VK_SUCCESS, "Failed to create render finished semaphore.");
			if (m_ImageRenderFinishedSemaphores[i] == VK_NULL_HANDLE)
				return false;
			const std::string renderFinName = "Image_Sema_RenderFin_" + iString;
			AURORA_VK_ATTACH_DEBUG_NAME(m_Specification.Device, VK_OBJECT_TYPE_SEMAPHORE, (uint64_t)m_ImageRenderFinishedSemaphores[i], "Swapchain_Sema_RenderFin_" + iString);
		}

		return true;
	}

	bool VulkanSwapchain::CreateRenderPass()
	{
		PROFILE_FUNCTION;

		VkAttachmentDescription colorAttachment{};
		colorAttachment.flags = 0;
		colorAttachment.format = m_ImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorRef{};
		colorRef.attachment = 0;
		colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.flags = 0;
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorRef;
		subpass.inputAttachmentCount = 0;
		subpass.pInputAttachments = nullptr;
		subpass.pDepthStencilAttachment = nullptr;
		subpass.pPreserveAttachments = nullptr;
		subpass.preserveAttachmentCount = 0;
		subpass.pResolveAttachments = nullptr;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo passInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
		passInfo.pNext = nullptr;
		passInfo.flags = 0;
		passInfo.attachmentCount = 1;
		passInfo.pAttachments = &colorAttachment;
		passInfo.dependencyCount = 1;
		passInfo.pDependencies = &dependency;
		passInfo.subpassCount = 1;
		passInfo.pSubpasses = &subpass;

		AURORA_VK_CHECK(vkCreateRenderPass(m_Specification.Device, &passInfo, m_Specification.AllocationCallbacks, &m_RenderPass), VK_SUCCESS, "Failed to create swapchain render pass");
		if (m_RenderPass == VK_NULL_HANDLE)
			return false;
		AURORA_VK_ATTACH_DEBUG_NAME(m_Specification.Device, VK_OBJECT_TYPE_RENDER_PASS, (uint64_t)m_RenderPass, "Swapchain_RenderPass");

		return true;
	}

	bool VulkanSwapchain::CreateFramebuffers()
	{
		PROFILE_FUNCTION;

		m_Framebuffers.resize(m_ImageViews.size());
		for (size_t i = 0; i < m_ImageViews.size(); i++)
		{
			VkImageView attachments[] =
			{
				m_ImageViews[i]
			};

			VkFramebufferCreateInfo fbInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
			fbInfo.pNext = nullptr;
			fbInfo.flags = 0;
			fbInfo.attachmentCount = 1;
			fbInfo.pAttachments = attachments;
			fbInfo.height = m_Extent.height;
			fbInfo.width = m_Extent.width;
			fbInfo.layers = 1;
			fbInfo.renderPass = m_RenderPass;
			AURORA_VK_CHECK(vkCreateFramebuffer(m_Specification.Device, &fbInfo, m_Specification.AllocationCallbacks, &m_Framebuffers[i]), VK_SUCCESS, "Failed to create swapchain framebuffer");
			if (m_Framebuffers[i] == VK_NULL_HANDLE)
				return false;
			AURORA_VK_ATTACH_DEBUG_NAME(m_Specification.Device, VK_OBJECT_TYPE_FRAMEBUFFER, (uint64_t)m_Framebuffers[i], "Swapchain_Framebuffer");
		}
		return true;
	}

	//========== Fallback ==========
	void VulkanSwapchain::RecordFallbackSwapchainRenderPass(const VulkanFrame& frame)
	{
		PROFILE_FUNCTION;

		VkRenderPassBeginInfo rpInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		rpInfo.pNext = nullptr;
		rpInfo.renderPass = m_RenderPass;
		rpInfo.framebuffer = m_Framebuffers[m_ImageIndex];
		rpInfo.renderArea.extent = m_Extent;
		rpInfo.renderArea.offset = { 0,0 };
		VkClearValue clearColor = { {{m_Specification.ClearColor.R, m_Specification.ClearColor.G, m_Specification.ClearColor.B, 1.0f}} };
		rpInfo.clearValueCount = 1;
		rpInfo.pClearValues = &clearColor;

		VkCommandBuffer cmd = frame.CommandBuffer;
		vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_FallbackPipeline);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_Extent.width);
		viewport.height = static_cast<float>(m_Extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(cmd, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = m_Extent;
		vkCmdSetScissor(cmd, 0, 1, &scissor);

		vkCmdDraw(cmd, 3, 1, 0, 0);

		vkCmdEndRenderPass(cmd);
	}

	bool VulkanSwapchain::CreateFallbackPipeline()
	{
		PROFILE_FUNCTION;

		//Shader Modules					
		VkShaderModuleCreateInfo vertInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
		vertInfo.pNext = nullptr;
		vertInfo.flags = 0;
		vertInfo.codeSize = Shaders::SwapchainFallback_vert_size;
		vertInfo.pCode = reinterpret_cast<const uint32_t*>(&Shaders::SwapchainFallback_vert);

		VkShaderModule vertModule = VK_NULL_HANDLE;
		AURORA_VK_CHECK(vkCreateShaderModule(m_Specification.Device, &vertInfo, nullptr, &vertModule), VK_SUCCESS, "Failed to create swapchain fallback vertex shader module.");
		if (vertModule == VK_NULL_HANDLE)
			return false;

		VkShaderModuleCreateInfo fragInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
		fragInfo.pNext = nullptr;
		fragInfo.flags = 0;
		fragInfo.codeSize = Shaders::SwapchainFallback_frag_size;
		fragInfo.pCode = reinterpret_cast<const uint32_t*>(&Shaders::SwapchainFallback_frag);

		VkShaderModule fragModule = VK_NULL_HANDLE;
		AURORA_VK_CHECK(vkCreateShaderModule(m_Specification.Device, &fragInfo, nullptr, &fragModule), VK_SUCCESS, "Failed to create swapchain fallback fragment shader module.");
		if (vertModule == VK_NULL_HANDLE)
			return false;


		//Shader stages
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
		vertShaderStageInfo.pNext = nullptr;
		vertShaderStageInfo.flags = 0;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertModule;
		vertShaderStageInfo.pName = "main";
		vertShaderStageInfo.pSpecializationInfo = nullptr;

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
		fragShaderStageInfo.pNext = nullptr;
		fragShaderStageInfo.flags = 0;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragModule;
		fragShaderStageInfo.pName = "main";
		fragShaderStageInfo.pSpecializationInfo = nullptr;

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		//Vertex input state
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		vertexInputInfo.pNext = nullptr;
		vertexInputInfo.flags = 0;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr;

		//Input assembly
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		inputAssemblyInfo.pNext = nullptr;
		inputAssemblyInfo.flags = 0;
		inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

		//Viewport
		// Not used (dynamic state)
		//VkViewport viewport{};
		//viewport.x = 0.0f;
		//viewport.y = 0.0f;
		//viewport.width = (float)m_Extent.width;
		//viewport.height = (float)m_Extent.height;
		//viewport.minDepth = 0.0f;
		//viewport.maxDepth = 1.0f;

		////Scissors
		//VkRect2D scissor{};
		//scissor.offset = { 0, 0 };
		//scissor.extent = m_Extent;

		//Danymic state
		std::vector<VkDynamicState> dynamicStates =
		{
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicStateInfo{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
		dynamicStateInfo.pNext = nullptr;
		dynamicStateInfo.flags = 0;
		dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicStateInfo.pDynamicStates = dynamicStates.data();

		VkPipelineViewportStateCreateInfo viewportInfo{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
		viewportInfo.pNext = nullptr;
		viewportInfo.flags = 0;
		viewportInfo.scissorCount = 1;
		viewportInfo.viewportCount = 1;

		//Rasterization state
		VkPipelineRasterizationStateCreateInfo rasterizationInfo{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
		rasterizationInfo.pNext = nullptr;
		rasterizationInfo.flags = 0;
		rasterizationInfo.depthClampEnable = VK_FALSE;
		rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
		rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationInfo.lineWidth = 1.0f;
		rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationInfo.depthBiasEnable = VK_FALSE;
		rasterizationInfo.depthBiasConstantFactor = 0.0f;
		rasterizationInfo.depthBiasClamp = 0.0f;
		rasterizationInfo.depthBiasSlopeFactor = 0.0f;

		//Multisampling state
		VkPipelineMultisampleStateCreateInfo multiSampInfo{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
		multiSampInfo.pNext = nullptr;
		multiSampInfo.flags = 0;
		multiSampInfo.sampleShadingEnable = VK_FALSE;
		multiSampInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multiSampInfo.minSampleShading = 1.0f;
		multiSampInfo.pSampleMask = nullptr;
		multiSampInfo.alphaToCoverageEnable = VK_FALSE;
		multiSampInfo.alphaToOneEnable = VK_FALSE;

		//DepthTesting (not used)
		//VkPipelineDepthStencilStateCreateInfo depthStencilInfo{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
		//depthStencilInfo.pNext = nullptr;
		//depthStencilInfo.flags = 0;
		//...

		//Color blending
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlendStateInfo{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
		colorBlendStateInfo.pNext = nullptr;
		colorBlendStateInfo.flags = 0;
		colorBlendStateInfo.logicOpEnable = VK_FALSE;
		colorBlendStateInfo.logicOp = VK_LOGIC_OP_COPY;
		colorBlendStateInfo.attachmentCount = 1;
		colorBlendStateInfo.pAttachments = &colorBlendAttachment;
		colorBlendStateInfo.blendConstants[0] = 0.0f;
		colorBlendStateInfo.blendConstants[1] = 0.0f;
		colorBlendStateInfo.blendConstants[2] = 0.0f;
		colorBlendStateInfo.blendConstants[3] = 0.0f;

		//Pipeline layout
		VkPipelineLayoutCreateInfo layoutInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
		layoutInfo.pNext = nullptr;
		layoutInfo.flags = 0;
		layoutInfo.setLayoutCount = 0;
		layoutInfo.pSetLayouts = nullptr;
		layoutInfo.pushConstantRangeCount = 0;
		layoutInfo.pPushConstantRanges = nullptr;

		AURORA_VK_CHECK(vkCreatePipelineLayout(m_Specification.Device, &layoutInfo, nullptr, &m_FallbackPipelineLayout), VK_SUCCESS, "Failed to create swapchain fallback pipeline layout.");
		if (m_FallbackPipelineLayout == VK_NULL_HANDLE)
			return false;
		AURORA_VK_ATTACH_DEBUG_NAME(m_Specification.Device, VK_OBJECT_TYPE_PIPELINE_LAYOUT, (uint64_t)m_FallbackPipelineLayout, "Swapchain_Fallback_PipelineLayout");

		//Pipeline
		VkGraphicsPipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
		pipelineInfo.pNext = nullptr;
		pipelineInfo.flags = 0;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
		pipelineInfo.pViewportState = &viewportInfo;
		pipelineInfo.pRasterizationState = &rasterizationInfo;
		pipelineInfo.pMultisampleState = &multiSampInfo;
		pipelineInfo.pDepthStencilState = nullptr;
		pipelineInfo.pColorBlendState = &colorBlendStateInfo;
		pipelineInfo.pDynamicState = &dynamicStateInfo;
		pipelineInfo.layout = m_FallbackPipelineLayout;
		pipelineInfo.renderPass = m_RenderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		AURORA_VK_CHECK(vkCreateGraphicsPipelines(m_Specification.Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_FallbackPipeline), VK_SUCCESS, "Failed to create swapchain fallback pipeline.");
		if (m_FallbackPipeline == VK_NULL_HANDLE)
			return false;
		AURORA_VK_ATTACH_DEBUG_NAME(m_Specification.Device, VK_OBJECT_TYPE_PIPELINE, (uint64_t)m_FallbackPipeline, "Swapchain_Fallback_Pipeline");

		vkDestroyShaderModule(m_Specification.Device, vertModule, nullptr);
		vkDestroyShaderModule(m_Specification.Device, fragModule, nullptr);

		return true;
	}



}