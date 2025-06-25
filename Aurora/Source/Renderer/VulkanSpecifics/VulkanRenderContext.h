#pragma once
#include "Aurora/Renderer/RenderContext.h"

#include "Renderer/VulkanSpecifics/DataStructs/PhysicalDeviceLimits.h"
#include "Renderer/VulkanSpecifics/Swapchain.h"
#include "Renderer/VulkanSpecifics/VulkanCore.h"

#include <vector>

namespace Aurora::VK {

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
		virtual void BeginFrame() override;
		virtual void EndFrame() override;
		virtual void SwapFrame() override;
		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual void Destroy() override;

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

	private:
		RenderContextSpecification m_Specification;

		VkInstance m_Instance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
		
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice m_Device = VK_NULL_HANDLE;
		VkCommandPool m_GraphicsCmdPool = VK_NULL_HANDLE;

		QueueFamilies m_QueueFamilies{};

		PhysicalDeviceLimits m_PhDeviceLimits{};

		Ref<Swapchain> m_Swapchain = nullptr;
	};

}