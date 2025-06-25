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

		struct ClearColor
		{
			float R = 0.5f;
			float G = 0.0f;
			float B = 0.0f;
			float A = 1.0f;
		} ClearColor;
	};

	struct FrameData
	{
		VkCommandBuffer CommandBuffer;
		uint8_t FrameIndex;
		uint64_t TotalFrameCount;
		VkExtent2D Extent;
	};

	class Swapchain
	{
	public:
		Swapchain(const SwapchainSpecification& spec);
		~Swapchain() = default;

		void Init();
		void PrepareFrame();
		const FrameData* AcquireNextFrameData();				
		void SwapImages();
		void FinalizeFrame();
		void OnResize(uint32_t width, uint32_t height);
		void Destroy();

		inline const SwapchainSpecification& GetSpecification() const { return m_Specification; }
		inline SwapchainSpecification& GetSpecification() { return m_Specification; }
		inline VkSwapchainKHR GetHandle() const { return m_Swapchain; }	

		// Fallback 

		/// <summary>
		/// Use this function as fallback if something breaks down somewhere else during development.
		/// Draws just the clear color.
		/// </summary>
		void RecordFallbackSwapchainRenderPass();

	private:
		void Submit();
		void Present();
		
		bool CreateSwapchain();
		bool CreateImageViews();
		bool CreatePipeline();
		bool CreateRenderPass();
		bool CreateFramebuffers();
		bool AllocateCommandBuffers();
		bool CreateSyncObjects();
		bool InitializeFrames();

		//For Fallback:
		bool CreateFallbackPipeline();

	private:
		SwapchainSpecification m_Specification{};
		VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
		VkRenderPass m_RenderPass = VK_NULL_HANDLE;

		VkFormat m_ImageFormat = VK_FORMAT_UNDEFINED;
		VkExtent2D m_Extent{};
				
		//Per image data
		uint32_t m_ImageIndex = 0;
		std::vector<VkImage> m_Images;
		std::vector<VkImageView> m_ImageViews;
		std::vector<VkFramebuffer> m_Framebuffers;

		//Todo: should be moved into main renderer (totalFrames and frame index) and passed into PrepareNextFrame when a basic renderer is established
		//Initialize swapchain with this and flow over to 0 the first time 'PrepareNextFrame' is called
		uint64_t m_TotalFinishedFrames = UINT64_MAX;
		
		//per frame data
		uint64_t m_FramesInFlightIdx = UINT64_MAX;
		std::vector<VkSemaphore> m_ImageAvailableSemaphores;
		std::vector<VkSemaphore> m_RenderFinishedSemaphores;
		std::vector<VkFence> m_InFlightFences;
		std::vector<VkCommandBuffer> m_CommandBuffers;
		std::vector<FrameData> m_FramesInFlight;
				
		//Fallback rendering only
		VkPipelineLayout m_FallbackPipelineLayout = VK_NULL_HANDLE;
		VkPipeline m_FallbackPipeline = VK_NULL_HANDLE;
	};


}