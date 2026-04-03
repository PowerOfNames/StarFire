#pragma once

#include "Aurora/Renderer/VulkanCore.h"

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

		struct SwapExtent
		{
			uint32_t Width;
			uint32_t Height;
		} InitialExtent;

		struct SwapClearColor
		{
			float R = 0.0f;
			float G = 0.0f;
			float B = 0.0f;
			float A = 1.0f;
		} ClearColor;
	};

	struct FrameData
	{
		VkCommandBuffer CommandBuffer = VK_NULL_HANDLE;
		uint8_t FrameIndex = 0;
		//The index of all frames. Inclusive (When this is the 5th frame ever rendered, this is 5)
		uint64_t FrameCount = 0;
		VkExtent2D Extent{};

		bool IsReady = false;
	};

	class Swapchain : public Substrate::RefCounted
	{
	public:
		Swapchain(const SwapchainSpecification& spec);
		~Swapchain() = default;

		void Init();
		bool PrepareFrame(uint32_t framesInFlightIdx);		
		bool SwapImages();
		void OnResize(uint32_t width, uint32_t height);
		void Destroy();

		inline const FrameData* GetCurrentFrameData() const { return &m_FramesInFlight[m_FramesInFlightIdx]; }
		inline const SwapchainSpecification& GetSpecification() const { return m_Specification; }
		inline SwapchainSpecification& GetSpecification() { return m_Specification; }
		inline VkSwapchainKHR GetHandle() const { return m_Swapchain; }	

		static Ref<Swapchain> Create(const SwapchainSpecification& spec);

		// Fallback 

		/// <summary>
		/// Use this function as fallback if something breaks down somewhere else during development.
		/// Draws just the clear color.
		/// </summary>
		void RecordFallbackSwapchainRenderPass();

	private:
		void CleanupSwapchain();
		void AcquireNextFrameData();
		void Submit();
		bool Present();

		bool CreateSwapchain(uint32_t width, uint32_t height);
		bool CreateImageViews();
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

		bool m_NeedsResize = false;

		VkFormat m_ImageFormat = VK_FORMAT_UNDEFINED;
		VkExtent2D m_Extent{};
				
		//Per image data
		uint32_t m_ImageIndex = 0;
		std::vector<VkImage> m_Images;
		std::vector<VkImageView> m_ImageViews;
		std::vector<VkFramebuffer> m_Framebuffers;
				
		//per frame data
		uint8_t m_FramesInFlightIdx = UINT8_MAX;
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