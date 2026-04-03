#pragma once
#include "Aurora/Renderer/VulkanCore.h"
#include "Aurora/Renderer/RenderContextSpecification.h"
#include "Aurora/Renderer/DataStructs/PhysicalDeviceLimits.h"
#include "Aurora/Renderer/Swapchain.h"

#include "Substrate/RefCounted.h"

#include <vector>

namespace Aurora::VK {

	struct DeviceRequirements {};

	struct QueueFamilies
	{
		VkQueue Graphics = VK_NULL_HANDLE;
		VkQueue Present = VK_NULL_HANDLE;
		VkQueue Compute = VK_NULL_HANDLE;

		//Move this out when time into transfer operation handler
		VkQueue Transfer = VK_NULL_HANDLE;
	};
		
	class RenderContext : public Substrate::RefCounted
	{
	public:
		RenderContext(const RenderContextSpecification& specs);
		~RenderContext() = default;

		void Init();
		bool BeginFrame();
		void EndFrame();
		void SwapFrame();
		void Resize(uint32_t width, uint32_t height);
		void Destroy();

		inline const RenderContextSpecification& GetSpecification() const { return m_Specification; }

		static Ref<RenderContext> Create(const RenderContextSpecification& specs);

	private:
		bool CreateInstance(
			const std::string& appName,
			const RenderContextSpecification::InstanceSpecification instanceSpecs,
			RenderContextSpecification::ApplicationVersionNumber appVersion,
			RenderContextSpecification::AuroraVersionNumber auroraVersion,
			WSIPlatformType wsi);

		bool CreateSurface(const RenderContextSpecification::SurfaceSpecification& surfaceSpecs);
		bool PickPhysicalDevice(const DeviceRequirements& deviceRequirements);
		bool CreateLogicalDevice(const DeviceRequirements& deviceRequirements);
		bool CreateVmAllocator();
		bool CreateGraphicsCmdPool();
		bool CreateSwapchain(const RenderContextSpecification::SurfaceSpecification& surfaceSpecs);


		//keep scoring up-to-date later (#76)
		int EvaluatePhysicalDevice(VkPhysicalDevice phDevice, const DeviceRequirements& deviceRequirements) const;
		bool CheckRequiredDeviceExtensionSupport(VkPhysicalDevice phDevice, const DeviceRequirements& deviceRequirements) const;
		const std::vector<const char*> GetRequiredDeviceExtensions(const DeviceRequirements& deviceRequirements) const;

		bool CheckRequiredLayerSupport(const std::vector<const char*>& requiredLayers) const;
		std::vector<const char*> GetRequiredInstanceExtensions(WSIPlatformType wsi) const;
		bool CheckRequiredInstanceExtensionsSupport(const std::vector<const char*>& requiredExtensions) const;

		//DebugName
		void SetupDebugMessenger(VkInstance instance, bool allowInfoLevel = false);
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo, bool allowInfoLevel = false);

		void IncrementFramesInFlightIdx();		

	private:
		RenderContextSpecification m_Specification;

		VkInstance m_Instance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
		
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice m_Device = VK_NULL_HANDLE;
		VkCommandPool m_GraphicsCmdPool = VK_NULL_HANDLE;

		VmaAllocator m_VmAllocator = VK_NULL_HANDLE;

		QueueFamilies m_QueueFamilies{};

		PhysicalDeviceLimits m_PhDeviceLimits{};

		Ref<Swapchain> m_Swapchain = nullptr;

		struct RendererStatistics
		{
			uint32_t FramesInFlightIdx = 0;
			uint64_t TotalAttemptedFrames = 0;
			uint64_t m_TotalFinishedFrames = 0;
		};
		RendererStatistics m_RendererStatistics{};
	};

}