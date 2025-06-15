#pragma once

#include "VulkanCore.h"

#include <vector>


namespace Aurora::VK {

	struct SwapchainSpecification
	{
		VkSurfaceKHR Surface = VK_NULL_HANDLE;
		VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
		VkDevice Device = VK_NULL_HANDLE;

		uint32_t FramesInFlight = 1;

		bool VSync = true;

		struct Extent
		{
			uint32_t Width;
			uint32_t Height;
		} InitialExtent;
	};

	struct SwapchainSupportDetails
	{
		VkSurfaceCapabilitiesKHR Capabilities;
		std::vector<VkSurfaceFormatKHR> Formats;
		std::vector<VkPresentModeKHR> PresentModes;
	};

	class Swapchain
	{
	public:
		Swapchain(const SwapchainSpecification& spec);
		~Swapchain() = default;

		void Init();
		void Destroy();


		inline const SwapchainSpecification& GetSpecification() const { return m_Specification; }
		inline SwapchainSpecification& GetSpecification() { return m_Specification; }
		inline VkSwapchainKHR GetHandle() const { return m_Swapchain; }

		static const SwapchainSupportDetails GetSupportDetails(VkPhysicalDevice phDevice, VkSurfaceKHR surface);
		static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats, VkFormat preferredFormat, VkColorSpaceKHR preferredColorSpace);
		static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availableModes, VkPresentModeKHR preferred);

		/// <summary>
		/// Method to query the swap extent. If currentExtent is set to UINT32_MAX, use framebufferWidth/Height clamped to min/max image extent.
		/// </summary>
		/// <param name="capabilities"></param>
		/// <param name="framebufferWidth">In pixels e.g. from glfwGetFramebufferSize</param>
		/// <param name="framebufferHeight">In pixels e.g. from glfwGetFramebufferSize</param>
		/// <returns></returns>
		static VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t framebufferWidth, uint32_t framebufferHeight);

	private:
		bool CreateSwapchain();
		bool CreateImageViews();
		bool CreatePipeline();
		bool CreateRenderPass();
		bool CreateFramebuffers();
	private:
		SwapchainSpecification m_Specification;
		VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
		VkRenderPass m_RenderPass = VK_NULL_HANDLE;
		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;

		VkFormat m_SwapchainImageFormat;
		VkExtent2D m_SwapchainExtent;
		
		std::vector<VkImage> m_SwapchainImages;
		std::vector<VkImageView> m_SwapchainImageViews;
		std::vector<VkFramebuffer> m_SwapchainFramebuffers;
	};


}