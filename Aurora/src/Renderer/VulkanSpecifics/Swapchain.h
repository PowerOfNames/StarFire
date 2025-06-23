#pragma once

#include "VulkanCore.h"

#include <vector>


namespace Aurora::VK {

	struct SwapchainSpecification
	{
		VkSurfaceKHR Surface = VK_NULL_HANDLE;
		VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
		VkDevice Device = VK_NULL_HANDLE;

		VkCommandPool GraphicsCmdPool = VK_NULL_HANDLE;
		VkQueue GraphicsQueue = VK_NULL_HANDLE;
		VkQueue PresentQueue = VK_NULL_HANDLE;

		uint32_t FramesInFlight = 1;

		bool VSync = true;

		struct Extent
		{
			uint32_t Width;
			uint32_t Height;
		} InitialExtent;
	};

	struct FrameData
	{
		VkCommandBuffer CommandBuffer;
		uint64_t FrameIndex;
		VkExtent2D Extent;
	};

	class Swapchain
	{
	public:
		Swapchain(const SwapchainSpecification& spec);
		~Swapchain() = default;

		void Init();
		const FrameData* AcquireNextFrame();
		void SwapImages();
		void OnResize(uint32_t width, uint32_t height);
		void Destroy();

		inline const SwapchainSpecification& GetSpecification() const { return m_Specification; }
		inline SwapchainSpecification& GetSpecification() { return m_Specification; }
		inline VkSwapchainKHR GetHandle() const { return m_Swapchain; }	

	private:
		bool CreateSwapchain();
		bool CreateImageViews();
		bool CreatePipeline();
		bool CreateRenderPass();
		bool CreateFramebuffers();
		bool AllocateCommandBuffers();
		bool CreateSyncObjects();
		bool InitializeFrames();

		void Submit();
		void Present();
	private:
		SwapchainSpecification m_Specification{};
		VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
		VkRenderPass m_RenderPass = VK_NULL_HANDLE;
		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;

		VkFormat m_ImageFormat = VK_FORMAT_UNDEFINED;
		VkExtent2D m_Extent{};
		
		uint32_t m_ImageIndex = 0;
		//Per image data
		std::vector<VkImage> m_Images;
		std::vector<VkImageView> m_ImageViews;
		std::vector<VkFramebuffer> m_Framebuffers;

		uint64_t m_TotalFrames = 0;
		uint64_t m_FrameIndex = 0;
		//per frame data
		std::vector<VkSemaphore> m_ImageAvailableSemaphores;
		std::vector<VkSemaphore> m_RenderFinishedSemaphores;
		std::vector<VkFence> m_InFlightFences;
		std::vector<VkCommandBuffer> m_CommandBuffers;
		std::vector<FrameData> m_FramesInFlight;
	};


}