#pragma once

#include "Aurora/Renderer/Vulkan/VulkanCore.h"
#include "Aurora/Renderer/Vulkan/Utility/VulkanHelper.h"

#include <vector>


namespace Aurora::VK {

	struct SwapchainSpecification
	{
		VkSurfaceKHR Surface = VK_NULL_HANDLE;
		VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
		VkDevice Device = VK_NULL_HANDLE;
		const VkAllocationCallbacks* AllocationCallbacks = nullptr;

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

	class VulkanSwapchain : public ::Substrate::RefCounted
	{
	public:
		VulkanSwapchain(const SwapchainSpecification& spec);
		~VulkanSwapchain() = default;

		void Init();
		bool PrepareFrame(VulkanFrame& frame);
		bool SwapImages(VulkanFrame& frame);
		void OnResize(uint32_t width, uint32_t height);
		void Destroy();

		inline const SwapchainSpecification& GetSpecification() const { return m_Specification; }
		inline SwapchainSpecification& GetSpecification() { return m_Specification; }
		inline VkSwapchainKHR GetHandle() const { return m_Swapchain; }	

		inline uint32_t GetImageCount() const { return static_cast<uint32_t>(m_Images.size()); }
		inline VkFormat GetImageFormat() const { return m_ImageFormat; }
		inline VkExtent2D GetExtent() const { return m_Extent; }
		inline const SwapchainSupportDetails GetSupportDetails() const { return Helper::GetSwapSupportDetails(m_Specification.PhysicalDevice, m_Specification.Surface); }

		static Ref<VulkanSwapchain> Create(const SwapchainSpecification& spec);

		// Fallback 

		/// <summary>
		/// Use this function as fallback if something breaks down somewhere else during development.
		/// Draws just the clear color.
		/// </summary>
		void RecordFallbackSwapchainRenderPass(const VulkanFrame& frame);

	private:
		void CleanupSwapchain();

		bool CreateSwapchain(uint32_t width, uint32_t height);
		bool CreateImageViews();
		bool CreateRenderPass();
		bool CreateFramebuffers();

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
				
		//Fallback rendering only
		VkPipelineLayout m_FallbackPipelineLayout = VK_NULL_HANDLE;
		VkPipeline m_FallbackPipeline = VK_NULL_HANDLE;
	};


}