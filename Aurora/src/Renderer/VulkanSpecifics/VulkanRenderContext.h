#pragma once
#include "Aurora/Renderer/RenderContext.h"

#include "Renderer/VulkanSpecifics/VulkanCore.h"

#include <vector>

namespace Aurora { namespace VK {

	struct DeviceRequirements
	{
		
	};

	struct QueueFamilies
	{
		VkQueue Graphics = VK_NULL_HANDLE;
		VkQueue Present = VK_NULL_HANDLE;
		VkQueue Compute = VK_NULL_HANDLE;
		//Move this out when time into transfer operation handler
		VkQueue Transfer = VK_NULL_HANDLE;
	};
		
	class VulkanRenderContext : public RenderContext
	{
	public:
		VulkanRenderContext(const RenderContextSpecification& specs);
		~VulkanRenderContext() = default;

		virtual void Init() override;
		virtual void Shutdown() override;

		inline virtual const RenderContextSpecification& GetSpecification() const override { return m_Specification; }

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

		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice phDevice);
		//keep scoring up-to-date later (#76)
		int EvaluatePhysicalDevice(VkPhysicalDevice phDevice, const DeviceRequirements& deviceRequirements);
		bool CheckRequiredDeviceExtensionSupport(VkPhysicalDevice phDevice, const DeviceRequirements& deviceRequirements);

		bool CheckRequiredLayerSupport(const std::vector<const char*>& requiredLayers);
		std::vector<const char*> GetRequiredInstanceExtensions(WSIPlatformType wsi);
		bool CheckRequiredInstanceExtensionsSupport(const std::vector<const char*>& requiredExtensions);

		//DebugName
		void SetupDebugMessenger(VkInstance instance, bool allowInfoLevel = false);
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo, bool allowInfoLevel = false);

	private:
		RenderContextSpecification m_Specification;

		VkInstance m_Instance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
		
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice m_Device = VK_NULL_HANDLE;
				
		QueueFamilies m_QueueFamilies{};

		PhysicalDeviceLimits m_PhDeviceLimits{};

		Ref<VkSwapchainKHR> m_Swapchain = nullptr;
	};

}
}