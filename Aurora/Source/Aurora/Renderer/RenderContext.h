#pragma once
#include "Aurora/Renderer/VulkanCore.h"
#include "Aurora/Renderer/DataStructs/PhysicalDeviceLimits.h"
#include "Aurora/Renderer/DataStructs/FrameData.h"
#include "Aurora/Renderer/DeletionQueue.h"
#include "Aurora/Renderer/RenderContextSpecification.h"
#include "Aurora/Renderer/Swapchain.h"
#include "Aurora/Renderer/VulkanHelper.h"


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

		inline void SubmitToMainDeletionQueue(std::function<void()> func)
		{
			m_MainDeletionQueue.SubmitDeletion(func);
		}
		inline void FlushMainDeletionQueue()
		{
			m_MainDeletionQueue.FlushDeletions();
		}

		inline void SubmitToFrameDeletionQueue(uint8_t frameIdx, std::function<void()> func)
		{
			GetCurrentFrameData().DeletionQueue.SubmitDeletion(func);
		}

		inline const RenderContextSpecification& GetSpecification() const { return m_Specification; }
		
		inline const FrameData& GetFrameData(uint8_t frameIdx) const { return m_FramesInFlight[frameIdx]; }
		inline FrameData& GetFrameData(uint8_t frameIdx) { return m_FramesInFlight[frameIdx]; }
		inline const FrameData& GetCurrentFrameData() const { return m_FramesInFlight[m_RendererStatistics.FramesInFlightIdx]; }
		inline FrameData& GetCurrentFrameData() { return m_FramesInFlight[m_RendererStatistics.FramesInFlightIdx]; }

		
		inline VkInstance GetVulkanInstance() const { return m_Instance; }
		inline VkPhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }
		inline const PhysicalDeviceLimits& GetPhysicalDeviceLimits() const { return m_PhDeviceLimits; }
		inline VkDevice GetLogicalDevice() const { return m_Device; }
		inline const QueueFamilies& GetQueueFamilies() const { return m_QueueFamilies; }
		inline const QueueFamilyIndices GetQueueFamilyIndices() const { return Helper::FindQueueFamilies(m_PhysicalDevice, m_Surface); }
		inline const VmaAllocator& GetVmaAllocator() const { return m_VmAllocator; }
		inline const VkApplicationInfo& GetApplicationInfo() const { return m_AppInfo; }
		inline const VkAllocationCallbacks* GetAllocationCallbacks() const { return m_AllocationCallbacks; }

		inline VkSurfaceKHR GetSurface() const { return m_Surface; }
		inline Ref<Swapchain> GetSwapchain() const { return m_Swapchain; }

		inline void SetImGuiActivity(bool state) { m_ImGuiInitialized = state; }

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
		bool CreateFramesInFlight(uint8_t framesInFlight);
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
		VkAllocationCallbacks* m_AllocationCallbacks = nullptr;

		VkApplicationInfo m_AppInfo{};

		QueueFamilies m_QueueFamilies{};

		PhysicalDeviceLimits m_PhDeviceLimits{};

		Ref<Swapchain> m_Swapchain = nullptr;
		std::vector<FrameData> m_FramesInFlight;
		

		DeletionQueue m_MainDeletionQueue{};

		struct RendererStatistics
		{
			uint32_t FramesInFlightIdx = 0;
			uint64_t TotalAttemptedFrames = 0;
			uint64_t m_TotalFinishedFrames = 0;
		};
		RendererStatistics m_RendererStatistics{};

		bool m_ImGuiInitialized = false;
	};

}