#pragma once
#include "Aurora/Renderer/Vulkan/VulkanCore.h"
#include "Aurora/Renderer/DeletionQueue.h"
#include "Aurora/Renderer/RenderContextSpecification.h"
#include "Aurora/Renderer/SubmissionOps.h"

#include "Aurora/Renderer/Vulkan/VulkanSwapchain.h"
#include "Aurora/Renderer/Vulkan/Utility/VulkanHelper.h"

#include "Substrate/RefCounted.h"

#include <vector>
#include <functional>

namespace Aurora::VK {
		
	using RenderCommand = std::function<void(VkCommandBuffer cmd)>;

	//TODO: refactor out:
	struct SubmitSpecifications
	{
		QueueOwner Queue = QueueOwner::GRAPHICS;
		std::vector<TimelineSemaphore> AdditionalWaitSemaphores;
		std::vector<TimelineSemaphore> AdditionalSignalSemaphores;
	};

	class VulkanContext : public Substrate::RefCounted
	{
	public:
		VulkanContext(const InitializationSpecification& specs);
		~VulkanContext() = default;

		void Init();
		bool BeginFrame();
		void Render();
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
			m_MainDeletionQueue.Flush(m_Device);
		}

		inline void SubmitToFrameDeletionQueue(std::function<void()> func, VkSemaphore semaphore = VK_NULL_HANDLE, uint64_t value = 0)
		{
			GetCurrentFrameData().DeletionQueue.SubmitDeletion(func, semaphore, value);
		}

		inline void SubmitRenderCommand(const RenderCommand&& cmd)
		{
			RenderCommandQueue.push_back(cmd);
		}

		void AddDeferredBufferCopySubmissionOps(const std::vector<VulkanBufferCopyOp>& ops, bool forceNow = false);
		void FlushDeferredSubmissionOps();

		void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& func, const SubmitSpecifications& specs = {});
		void CopyBufferToBuffer(BufferHandle src, BufferHandle dst, bool forceNow/* = false*/, bool destroySrc/* =true*/);

		
		inline const InitializationSpecification& GetSpecification() const { return m_Specification; }
		
		inline const VulkanFrame& GetFrameData(uint8_t frameIdx) const { return m_FramesInFlight[frameIdx]; }
		inline VulkanFrame& GetFrameData(uint8_t frameIdx) { return m_FramesInFlight[frameIdx]; }
		inline const VulkanFrame& GetCurrentFrameData() const { return m_FramesInFlight[m_RendererStatistics.FramesInFlightIdx]; }
		inline VulkanFrame& GetCurrentFrameData() { return m_FramesInFlight[m_RendererStatistics.FramesInFlightIdx]; }

		TimelineSemaphore GetQueueSemaphoreSnapshot(QueueOwner owner);
		uint32_t GetQueueFamilyIndexFromOwner(QueueOwner owner) const;
		VkQueue GetQueueFromOwner(QueueOwner owner) const;

		inline VkInstance GetVulkanInstance() const { return m_Instance; }
		inline VkPhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }
		inline const PhysicalDeviceLimits& GetPhysicalDeviceLimits() const { return m_PhDeviceLimits; }
		inline VkDevice GetLogicalDevice() const { return m_Device; }
		inline const QueueFamilies& GetQueueFamilies() const { return m_QueueFamilies; }
		inline const QueueFamilyIndices GetQueueFamilyIndices() const { return Helper::FindQueueFamilies(m_PhysicalDevice, m_Surface); }
		inline const VmaAllocator& GetVmaAllocator() const { return m_VmAllocator; }
		inline const VkApplicationInfo& GetApplicationInfo() const { return m_AppInfo; }
		inline const VkAllocationCallbacks* GetAllocationCallbacks() const { return m_AllocationCallbacks; }

		inline const VkCommandPool GetGraphicsCommandPool() const { return m_MainGraphicsCmdPool; }
		inline const VkCommandPool GetTransferCommandPool() const { return m_TransferCmdPool; }

		inline const VkDescriptorPool GetBindlessDescriptorPool() const { return m_BindlessDescriptorPool; }
		inline const VkDescriptorSetLayout GetBindlessDescriptorSetLayout() const { return m_BindlessDescriptorSetLayout; }
		inline const VkDescriptorSet GetBindlessDescriptorSet() const { return m_BindlessDescriptorSet; }

		inline const VkPipeline GetBindlessGraphicsPipeline() const { return m_BindlessGraphicsPipeline; }
		inline const VkPipelineLayout GetBindlessGraphicsPipelineLayout() const { return m_BindlessGraphicsPipelineLayout; }

		inline VkSurfaceKHR GetSurface() const { return m_Surface; }
		inline Ref<VulkanSwapchain> GetSwapchain() const { return m_Swapchain; }



		static Ref<VulkanContext> Create(const InitializationSpecification& specs);

	private:
		bool CreateInstance(
			const std::string& appName,
			const InitializationSpecification::InstanceSpecification instanceSpecs,
			InitializationSpecification::ApplicationVersionNumber appVersion,
			InitializationSpecification::AuroraVersionNumber auroraVersion,
			WSIPlatformType wsi);

		bool CreateSurface(const InitializationSpecification::SurfaceSpecification& surfaceSpecs);
		bool PickPhysicalDevice(const DeviceRequirements& deviceRequirements);
		bool CreateLogicalDevice(const DeviceRequirements& deviceRequirements);
		bool CreateVmAllocator();
		bool CreateGraphicsCommandPools();
		bool CreateTransferSubmissionStructures();
		bool CreateFramesInFlight(uint8_t framesInFlight);
		bool CreateBindlessDescriptorSet();
		bool CreateBindlessGraphicsPipeline();
		bool CreateSwapchain(const InitializationSpecification::SurfaceSpecification& surfaceSpecs);

		bool CheckTimelineSemaphore(VkSemaphore sema, uint64_t targetValue) const;


		// ===== SubmissionHelper =====
		// TODO: refactor out - needs to be public atm to grant access to SubmissionScheduler for immediate submit
		void HandleBufferCopySubmissionOp(const VulkanBufferCopyOp& op);
		TimelineSemaphore& GetSubmissionSemaFromQueueOwner(QueueOwner owner);
		VkCommandBuffer GetCommandBufferFromQueueOwner(QueueOwner owner);
		
		// ===== =====

		// == Frame management ==
		void FlushFrameDeletionQueue(uint8_t frameIdx);
		void IncrementFramesInFlightIdx();		
		
		
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
		InitializationSpecification m_Specification;

		VkInstance m_Instance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
		
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice m_Device = VK_NULL_HANDLE;

		VkCommandPool m_MainGraphicsCmdPool = VK_NULL_HANDLE;
		
		VkCommandPool m_TransferCmdPool = VK_NULL_HANDLE;
		VkCommandBuffer m_TransferCmdBuffer = VK_NULL_HANDLE;
		TimelineSemaphore m_TransferSubmitSemaphore{};
		
		VkCommandPool m_GraphicsTransferCmdPool = VK_NULL_HANDLE;
		VkCommandBuffer m_GraphicsTransferCmdBuffer = VK_NULL_HANDLE;
		TimelineSemaphore m_GraphicsSubmitSemaphore{};

		VkCommandPool m_ComputeTransferCmdPool = VK_NULL_HANDLE;
		VkCommandBuffer m_ComputeTransferCmdBuffer = VK_NULL_HANDLE;
		TimelineSemaphore m_ComputeSubmitSemaphore{};
		

		std::unordered_map<QueueOwner, uint32_t> m_QueueOwnerIndices;
		std::unordered_map<QueueOwner, VkQueue> m_QueueOwnerQueues;
		std::vector<VulkanBufferCopyOp> m_DeferredBufferCopySubmissionOps;

		VmaAllocator m_VmAllocator = VK_NULL_HANDLE;
		VkAllocationCallbacks* m_AllocationCallbacks = nullptr;

		VkApplicationInfo m_AppInfo{};

		VkDescriptorPool m_BindlessDescriptorPool = VK_NULL_HANDLE;
		VkDescriptorSetLayout m_BindlessDescriptorSetLayout = VK_NULL_HANDLE;
		VkDescriptorSet m_BindlessDescriptorSet = VK_NULL_HANDLE;

		VkPipelineLayout m_BindlessGraphicsPipelineLayout = VK_NULL_HANDLE;
		VkPipeline m_BindlessGraphicsPipeline = VK_NULL_HANDLE;

		QueueFamilies m_QueueFamilies{};
		
		

		PhysicalDeviceLimits m_PhDeviceLimits{};

		Ref<VulkanSwapchain> m_Swapchain = nullptr;
		std::vector<VulkanFrame> m_FramesInFlight;

		std::deque<RenderCommand> RenderCommandQueue;

		DeletionQueue m_MainDeletionQueue{};

		struct RendererStatistics
		{
			uint32_t FramesInFlightIdx = 0;
			uint64_t TotalAttemptedFrames = 0;
			uint64_t m_TotalFinishedFrames = 0;
		};
		RendererStatistics m_RendererStatistics{};
	};

}