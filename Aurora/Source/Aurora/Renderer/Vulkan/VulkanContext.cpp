#include "Aurora/Renderer/Vulkan/VulkanContext.h"

#include "Aurora/Core/Core.h"
#include "AuroraInternal.h"
#include "Aurora/Profiling/Profiling.h"
#include "Aurora/Renderer/Vulkan/Utility/VulkanToString.h"
#include "Aurora/Renderer/Vulkan/Utility/AuroraToVulkan.h"
#include "Aurora/Renderer/Vulkan/VulkanSubmissionScheduler.h"
#include "Aurora/Renderer/Vulkan/Utility/VulkanCreators.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <map>
#include <set>
#include <string>

namespace Aurora::VK {

	VulkanContext::VulkanContext(const InitializationSpecification& specs)
		: m_Specification(specs)
	{
	}

	Ref<VulkanContext> VulkanContext::Create(const InitializationSpecification& specs)
	{
		return CreateRef<VulkanContext>(specs);
	}


	void VulkanContext::Init()
	{
		PROFILE_FUNCTION;

		if (!CreateInstance(
			m_Specification.AppName,
			m_Specification.InstanceSpecs,
			m_Specification.AppVersion,
			m_Specification.AuroraVersion,
			m_Specification.SurfaceSpecs.WSI))
		{
			AURORA_ERROR("Failed to create instance. VulkanContext could not be initialized.");
			return;
		}


		if (!CreateSurface(m_Specification.SurfaceSpecs))
		{
			AURORA_ERROR("Failed to create surface. VulkanContext could not be instanziated.");
			return;
		}

		DeviceRequirements deviceReqs{};
		if (!PickPhysicalDevice(deviceReqs))
		{
			AURORA_ERROR("Failed to pick a physical device. VulkanContext could not be instantiated.");
			return;
		}

		if (!CreateLogicalDevice(deviceReqs))
		{
			AURORA_ERROR("Failed to create logical device. VulkanContext could not be instantiated.");
			return;
		}

		if (!CreateVmAllocator())
		{
			AURORA_ERROR("Failed to create vmAllocator. VulkanContext could not be instantiated.");
			return;
		}

		if (!CreateGraphicsCommandPools())
		{
			AURORA_TRACE("Failed to create graphics command pool. VulkanContext could not be initialized.");
			return;
		}

		if (!CreateTransferSubmissionStructures())
		{
			AURORA_ERROR("Failed to create transfer submission structures. VulkanContext could not be initialized.");
			return;
		}

		if (!CreateFramesInFlight(m_Specification.SurfaceSpecs.FramesPerFlight))
		{
			AURORA_ERROR("Failed to create per-frame data. VulkanContext could not be initialized.");
			return;
		}		

		if (!CreateSwapchain(m_Specification.SurfaceSpecs))
		{
			AURORA_ERROR("Failed to create swapchain. VulkanContext could not be initialized.");
			return;
		}

		m_Renderer = CreateRef<VulkanRenderer>();
		if (m_Renderer == nullptr)
		{
			AURORA_ERROR("Failed to create renderer. VulkanContext could not be initialized.");
			return;
		}
		if (!m_Renderer->Init())
		{
			AURORA_ERROR("Failed to initialize renderer. VulkanContext could not be initialized.");
			return;
		}

		//This initializes FIF, such that the very first rendered frame still is index 0;
		m_RendererStatistics.FramesInFlightIdx = m_Specification.SurfaceSpecs.FramesPerFlight - 1;
		AURORA_INFO("Successfully initialized vulkan rendering context");
	}


	void VulkanContext::Destroy()
	{
		PROFILE_FUNCTION;

		AURORA_VK_CHECK(vkDeviceWaitIdle(m_Device), VK_SUCCESS, "RenderContext::Destroy: Failed to wait for device idle!");

		m_Renderer->Destroy();
		m_MainDeletionQueue.Flush(m_Device);
		m_FramesInFlight.clear();

		AURORA_INFO("Destroyed all vulkan context objects.");
	}


	bool VulkanContext::BeginFrame()
	{
		PROFILE_FUNCTION;

		//Cleanup old frame
		FlushFrameDeletionQueue(m_RendererStatistics.FramesInFlightIdx);
		
		//Prepare next frame
		IncrementFramesInFlightIdx();

		AURORA_TRACE("Beginning frame {}", m_RendererStatistics.FramesInFlightIdx);
		VulkanFrame& frame = GetCurrentFrameData();
		if (!m_Swapchain->PrepareFrame(frame))
			return false;

		PollPendingResourceUploads();
		FlushDeferredSubmissionOps();

		VkCommandBufferBeginInfo cmdInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		cmdInfo.pNext = nullptr;
		cmdInfo.pInheritanceInfo = nullptr;
		cmdInfo.flags = 0;

		AURORA_VK_CHECK(vkBeginCommandBuffer(frame.CommandBuffer, &cmdInfo), VK_SUCCESS, "Failed to begin command buffer (frame index: {}).", frame.FrameIndex);

		//TEMP:
		//m_Swapchain->RecordFallbackSwapchainRenderPass(frame);

		return true;
	}

	void VulkanContext::EndFrame()
	{
		PROFILE_FUNCTION;


		VulkanFrame& frame = GetCurrentFrameData();

		AURORA_VK_CHECK(vkEndCommandBuffer(frame.CommandBuffer), VK_SUCCESS, "Failed to end command buffer (frame index: {}).", frame.FrameIndex);
	}

	void VulkanContext::Render()
	{
		PROFILE_FUNCTION;


		VkCommandBuffer cmd = GetCurrentFrameData().CommandBuffer;
		/*for (const RenderCommand& command : RenderCommandQueue)
		{
			command(cmd);
		}*/
	}

	void VulkanContext::SwapFrame()
	{
		PROFILE_FUNCTION;

		if (m_Swapchain->SwapImages(GetCurrentFrameData()))
			m_RendererStatistics.m_TotalFinishedFrames++;
	}

	void VulkanContext::Resize(uint32_t width, uint32_t height)
	{
		PROFILE_FUNCTION;

		m_Swapchain->OnResize(width, height);
	}

	// ========== Object Creation ==========
	bool VulkanContext::CreateInstance(
		const std::string& appName,
		const InitializationSpecification::InstanceSpecification instanceSpecs,
		InitializationSpecification::ApplicationVersionNumber appVersion,
		InitializationSpecification::AuroraVersionNumber auroraVersion,
		WSIPlatformType wsi)
	{
		PROFILE_FUNCTION;


		// ===== Layers =====		
		//TODO: make this dynamic!
		const std::vector<const char*> requiredLayers =
		{
#if AURORA_VK_VALIDATION_ENABLED
			"VK_LAYER_KHRONOS_validation",
#endif
			"VK_LAYER_KHRONOS_synchronization2"
		};
		if (!CheckRequiredLayerSupport(requiredLayers))
		{
			AURORA_ERROR("Some required layers are not available! Instance will not be created.");
			return false;
		}

		// ===== Extensions =====
		std::vector<const char*> requiredExtensions = GetRequiredInstanceExtensions(wsi);

#if AURORA_VK_VALIDATION_ENABLED
		bool useDebugUtils = instanceSpecs.EnableDebugUtils;
#else
		bool useDebugUtils = false;
#endif 
		if (useDebugUtils)
			requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		if (!CheckRequiredInstanceExtensionsSupport(requiredExtensions))
		{
			AURORA_ERROR("Some required extensions are not available! Instance will not be created.");
			return false;
		}

		// ===== Application =====

		VkApplicationInfo appInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
		appInfo.pNext = nullptr;
		appInfo.pEngineName = "Aurora";
		appInfo.apiVersion = VK_API_VERSION_1_4;
		appInfo.pApplicationName = appName.c_str();
		appInfo.applicationVersion = VK_MAKE_VERSION(appVersion.Major, appVersion.Minor, appVersion.Patch);
		appInfo.engineVersion = VK_MAKE_VERSION(auroraVersion.Major, auroraVersion.Minor, auroraVersion.Patch);
		m_AppInfo = appInfo;

		// ===== Instance =====

		VkInstanceCreateInfo instanceInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
		if (useDebugUtils)
		{
			VkDebugUtilsMessengerCreateInfoEXT debugUtilsInfo;
			PopulateDebugMessengerCreateInfo(debugUtilsInfo, instanceSpecs.EnableDebugUtils);
			instanceInfo.pNext = static_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&debugUtilsInfo);
		}
		else
			instanceInfo.pNext = nullptr;
		instanceInfo.flags = 0;
		instanceInfo.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size());
		instanceInfo.ppEnabledLayerNames = requiredLayers.data();
		instanceInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
		instanceInfo.ppEnabledExtensionNames = requiredExtensions.data();
		instanceInfo.pApplicationInfo = &appInfo;

		AURORA_VK_CHECK(vkCreateInstance(&instanceInfo, m_AllocationCallbacks, &m_Instance), VK_SUCCESS, "Failed to create VkInstance.");
		if (m_Instance == VK_NULL_HANDLE)
		{
			AURORA_CRITICAL("Failed to create a Vulkan Instance!");
			return false;
		}
		AURORA_TRACE("Created VkInstance.");

		SubmitToMainDeletionQueue([this]()
			{
				vkDestroyInstance(m_Instance, m_AllocationCallbacks);
				m_Instance = VK_NULL_HANDLE;
			});

#if AURORA_VK_VALIDATION_ENABLED
		if (useDebugUtils)
			SetupDebugMessenger(m_Instance);
#endif // AURORA_VK_VALIDATION

		AURORA_TRACE("Created vulkan instance.");
		return true;
	}


	bool VulkanContext::CreateSurface(const InitializationSpecification::SurfaceSpecification& surfaceSpecs)
	{
		PROFILE_FUNCTION;

		switch (surfaceSpecs.WSI)
		{
			case WSIPlatformType::SURFACE_PLATFORM_GLFW:
			{
				AURORA_VK_CHECK(glfwCreateWindowSurface(m_Instance, (GLFWwindow*)surfaceSpecs.WindowHandle, m_AllocationCallbacks, &m_Surface), VK_SUCCESS, "Failed to create GLFWWindow Surface.");
				break;
			}
			case WSIPlatformType::SURFACE_PLAFORM_NONE:
			default:
			{
				AURORA_ERROR("WSI currently not supported!");
				return false;
			}
		}

		if (m_Surface == VK_NULL_HANDLE)
		{
			AURORA_CRITICAL("Failed to create a Vulkan Surface!");
			return false;
		}

		SubmitToMainDeletionQueue([this]()
			{
				vkDestroySurfaceKHR(m_Instance, m_Surface, m_AllocationCallbacks);
				m_Surface = VK_NULL_HANDLE;
			});

		AURORA_TRACE("Created vulkan surface.");
		return true;
	}

	bool VulkanContext::PickPhysicalDevice(const DeviceRequirements& deviceRequirements)
	{
		PROFILE_FUNCTION;

		uint32_t deviceCount;
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
		if (deviceCount == 0)
		{
			AURORA_CRITICAL("No devices found.");
			return false;
		}
		std::vector<VkPhysicalDevice> availableDevices(deviceCount);
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, availableDevices.data());

		std::multimap<int, VkPhysicalDevice> candidates;
		for (const auto& phDevice : availableDevices)
		{
			//Keep up-to-date (#76): 
			int score = EvaluatePhysicalDevice(phDevice, deviceRequirements);
			AURORA_TRACE("Found device with score {}", score);
			candidates.insert(std::make_pair(score, phDevice));
		}
		if (candidates.rbegin()->first <= 0)
		{
			AURORA_CRITICAL("No suitable device found.");
			return false;
		}
		m_PhysicalDevice = candidates.rbegin()->second;

		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(m_PhysicalDevice, &props);

		AURORA_INFO("Picked physical device properties");
		AURORA_INFO("==========================================================");
		AURORA_INFO("Name			: {}", props.deviceName);
		AURORA_INFO("Type			: {}", DeviceTypeToString(props.deviceType));
		AURORA_INFO("API Version	: {}", ApiVersionToString(props.apiVersion));
		AURORA_INFO("DeviceID		: {}", props.deviceID);
		AURORA_INFO("==========================================================\n\n");

		m_PhDeviceLimits.DescriptorLimits.MaxPerStageDescriptorInputAttachments = props.limits.maxPerStageDescriptorInputAttachments;
		m_PhDeviceLimits.DescriptorLimits.MaxPerStageDescriptorSampledImages = props.limits.maxPerStageDescriptorSampledImages;
		m_PhDeviceLimits.DescriptorLimits.MaxPerStageDescriptorStorageImages = props.limits.maxPerStageDescriptorStorageImages;
		m_PhDeviceLimits.DescriptorLimits.MaxPerStageDescriptorStorageBuffers = props.limits.maxPerStageDescriptorStorageBuffers;
		m_PhDeviceLimits.DescriptorLimits.MaxPerStageSamplers = props.limits.maxDescriptorSetSamplers;
		m_PhDeviceLimits.DescriptorLimits.MaxPerStageResources = props.limits.maxPerStageResources;

		AURORA_INFO("Physical device limits");
		AURORA_INFO("==========================================================");
		AURORA_INFO("Max Per Stage Input Attachments	: {}", m_PhDeviceLimits.DescriptorLimits.MaxPerStageDescriptorInputAttachments);
		AURORA_INFO("Max Per Stage Sampled Images		: {}", m_PhDeviceLimits.DescriptorLimits.MaxPerStageDescriptorSampledImages);
		AURORA_INFO("Max Per Stage Storage Images		: {}", m_PhDeviceLimits.DescriptorLimits.MaxPerStageDescriptorStorageImages);
		AURORA_INFO("Max Per Stage Storage Buffers		: {}", m_PhDeviceLimits.DescriptorLimits.MaxPerStageDescriptorStorageBuffers);
		AURORA_INFO("Max Per Stage Resources			: {}", m_PhDeviceLimits.DescriptorLimits.MaxPerStageResources);
		AURORA_INFO("========================================================== \n\n");

		AURORA_TRACE("Picked physical device");
		return true;
	}

	bool VulkanContext::CreateLogicalDevice(const DeviceRequirements& deviceRequirements)
	{
		PROFILE_FUNCTION;


		QueueFamilyIndices indices = Helper::FindQueueFamilies(m_PhysicalDevice, m_Surface);
		uint8_t uniqueQueueFamilies = indices.UniqueFamilyIndices();
		AURORA_INFO("Picked physical device queue families");
		AURORA_INFO("==========================================================");
		AURORA_INFO("Unique Queue Families	: {}", uniqueQueueFamilies);
		AURORA_INFO("Graphics		: {}", indices.Graphics);
		AURORA_INFO("Present		: {}", indices.Present);
		AURORA_INFO("Compute		: {}", indices.Compute);
		AURORA_INFO("Transfer		: {}", indices.Transfer);
		AURORA_INFO("========================================================== \n");

		std::vector<VkDeviceQueueCreateInfo> queueInfos;
		std::set<uint32_t> uniqueQueueFamilyIndices{ indices.Graphics, indices.Present, indices.Transfer, indices.Compute };
		queueInfos.reserve(uniqueQueueFamilyIndices.size());

		float priority = 1.0f;
		for (const auto& index : uniqueQueueFamilyIndices)
		{
			VkDeviceQueueCreateInfo queueInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
			queueInfo.pNext = nullptr;
			queueInfo.flags = 0;
			queueInfo.pQueuePriorities = &priority;
			//Integrated GPUs mosty have one queue family supporting all operations -> try to dedicate one index within this family
			queueInfo.queueCount = uniqueQueueFamilies == 1 ? indices.UnifiedCount : 1;
			queueInfo.queueFamilyIndex = index;
			queueInfos.push_back(queueInfo);
		}

		if (queueInfos.size() == 0)
		{
			AURORA_ERROR("No queue families can be created!");
			return false;
		}

		VkPhysicalDeviceFeatures features{};
		auto requiredExtension = GetRequiredDeviceExtensions(deviceRequirements);

		VkPhysicalDeviceVulkan12Features features12{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
		features12.bufferDeviceAddress = true;
		features12.descriptorIndexing = true;
		features12.runtimeDescriptorArray = true;
		features12.descriptorBindingPartiallyBound = true;
		features12.shaderStorageBufferArrayNonUniformIndexing = true;
		features12.shaderSampledImageArrayNonUniformIndexing = true;
		features12.shaderStorageImageArrayNonUniformIndexing = true;
		features12.timelineSemaphore = true;

		features12.descriptorBindingStorageBufferUpdateAfterBind = true;
		features12.descriptorBindingSampledImageUpdateAfterBind = true;
		features12.descriptorBindingStorageImageUpdateAfterBind = true;

		VkPhysicalDeviceVulkan13Features features13{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
		features13.pNext = &features12;
		features13.dynamicRendering = true;
		features13.synchronization2 = true;

		// to enamle VK_DEPENDENCY_QUEUE_FAMILY_OWNERSHIP_TRANSFER_USE_ALL_STAGES_BIT_KHR for more granular ownership transfer barriers
		VkPhysicalDeviceMaintenance8FeaturesKHR featureMaintenance8{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_8_FEATURES_KHR };
		featureMaintenance8.pNext = &features13;
		featureMaintenance8.maintenance8 = true;

		VkDeviceCreateInfo deviceInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
		deviceInfo.pNext = &featureMaintenance8;
		deviceInfo.flags = 0;
		deviceInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtension.size());
		deviceInfo.ppEnabledExtensionNames = requiredExtension.data();
		deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
		deviceInfo.pQueueCreateInfos = queueInfos.data();
		deviceInfo.pEnabledFeatures = &features;
		AURORA_VK_CHECK(vkCreateDevice(m_PhysicalDevice, &deviceInfo, m_AllocationCallbacks, &m_Device), VK_SUCCESS, "Failed to create device!");

		if (m_Device == VK_NULL_HANDLE)
		{
			AURORA_ERROR("Failed to create logical device!");
			return false;
		}

		//TODO: Trace print enabled features here:

		uint32_t finalGraphicsSlot = 0;
		vkGetDeviceQueue(m_Device, indices.Graphics, finalGraphicsSlot, &m_QueueFamilies.Graphics);
		AURORA_TRACE("Got graphics queue from queue family {} at slot {}", indices.Graphics, finalGraphicsSlot);
		uint32_t finalPresentSlot = 0;
		vkGetDeviceQueue(m_Device, indices.Present, finalPresentSlot, &m_QueueFamilies.Present);
		AURORA_TRACE("Got present queue from queue family {} at slot {}", indices.Present, finalPresentSlot);

		//if compute is not on a different queue family then graphics, try to get a non-graphics queue 
		// (graphics always on slot 0)
		uint32_t finalComputeSlot;
		if (indices.HasDedicatedCompute || indices.Compute != indices.Graphics)
			finalComputeSlot = 0;
		else
			finalComputeSlot = (indices.UnifiedCount + 1) % indices.UnifiedCount;
		vkGetDeviceQueue(m_Device, indices.Compute, finalComputeSlot, &m_QueueFamilies.Compute);
		AURORA_TRACE("Got compute queue from queue family {} at slot {}", indices.Compute, finalComputeSlot);

		uint32_t finalTransferSlot;
		if (indices.HasDedicatedCompute || indices.Transfer != indices.Graphics)
			finalTransferSlot = 0;
		else
			finalTransferSlot = indices.UnifiedCount > 2 ? 2 : finalComputeSlot;
		vkGetDeviceQueue(m_Device, indices.Transfer, finalTransferSlot, &m_QueueFamilies.Transfer);
		AURORA_TRACE("Got transfer queue from queue family {} at slot {}", indices.Transfer, finalTransferSlot);

		m_PhDeviceLimits.QueueFamLimits.HasDedicatedComputeQueue = indices.HasDedicatedCompute;
		m_PhDeviceLimits.QueueFamLimits.HasDedicatedTransferQueue = indices.HasDedicatedTransfer;

		m_QueueOwnerIndices[QueueOwner::UNKNOWN] = VK_QUEUE_FAMILY_IGNORED;
		m_QueueOwnerIndices[QueueOwner::PRESENT] = indices.Present;
		m_QueueOwnerIndices[QueueOwner::GRAPHICS] = indices.Graphics;
		m_QueueOwnerIndices[QueueOwner::COMPUTE] = indices.Compute;
		m_QueueOwnerIndices[QueueOwner::TRANSFER] = indices.Transfer;

		m_QueueOwnerQueues[QueueOwner::UNKNOWN] = VK_NULL_HANDLE;
		m_QueueOwnerQueues[QueueOwner::PRESENT] = m_QueueFamilies.Present;
		m_QueueOwnerQueues[QueueOwner::GRAPHICS] = m_QueueFamilies.Graphics;
		m_QueueOwnerQueues[QueueOwner::COMPUTE] = m_QueueFamilies.Compute;
		m_QueueOwnerQueues[QueueOwner::TRANSFER] = m_QueueFamilies.Transfer;

		SubmitToMainDeletionQueue([this]()
			{
				vkDestroyDevice(m_Device, m_AllocationCallbacks);
				m_Device = VK_NULL_HANDLE;
			});

		AURORA_TRACE("Created vulkan logical device and gathered queues");
		return true;
	}

	bool VulkanContext::CreateVmAllocator()
	{
		PROFILE_FUNCTION;

		VmaAllocatorCreateInfo alInfo{};
		alInfo.instance = m_Instance;
		alInfo.device = m_Device;
		alInfo.physicalDevice = m_PhysicalDevice;
		alInfo.vulkanApiVersion = VK_API_VERSION_1_3;
		alInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
		alInfo.pAllocationCallbacks = m_AllocationCallbacks;
		alInfo.pDeviceMemoryCallbacks = nullptr;
		alInfo.pHeapSizeLimit = nullptr;
		alInfo.pTypeExternalMemoryHandleTypes = nullptr;
		VmaVulkanFunctions vulkanFunctions{};
		vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
		vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;
		alInfo.pVulkanFunctions = &vulkanFunctions;
		vmaCreateAllocator(&alInfo, &m_VmAllocator);
		if (m_VmAllocator == VK_NULL_HANDLE)
			return false;

		SubmitToMainDeletionQueue([this]()
			{
				vmaDestroyAllocator(m_VmAllocator);
				m_VmAllocator = VK_NULL_HANDLE;
			});

		return true;
	}

	bool VulkanContext::CreateGraphicsCommandPools()
	{
		PROFILE_FUNCTION;

		VkCommandPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		poolInfo.pNext = nullptr;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = Helper::FindQueueFamilies(m_PhysicalDevice, m_Surface).Graphics;

		AURORA_VK_CHECK(vkCreateCommandPool(m_Device, &poolInfo, m_AllocationCallbacks, &m_MainGraphicsCmdPool), VK_SUCCESS, "Failed to create graphics command pool.");
		if (m_MainGraphicsCmdPool == VK_NULL_HANDLE)
			return false;
		AURORA_VK_ATTACH_DEBUG_NAME(m_Device, VK_OBJECT_TYPE_COMMAND_POOL, (uint64_t)m_MainGraphicsCmdPool, "GraphicsCommandPool");

		SubmitToMainDeletionQueue([this]()
			{
				vkDestroyCommandPool(m_Device, m_MainGraphicsCmdPool, m_AllocationCallbacks);
				m_MainGraphicsCmdPool = VK_NULL_HANDLE;
			});

		return true;
	}

	bool VulkanContext::CreateTransferSubmissionStructures()
	{
		PROFILE_FUNCTION;

		// ===== Transfer queue structures =====
		VkCommandPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		poolInfo.pNext = nullptr;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		poolInfo.queueFamilyIndex = Helper::FindQueueFamilies(m_PhysicalDevice, m_Surface).Transfer;

		AURORA_VK_CHECK(vkCreateCommandPool(m_Device, &poolInfo, m_AllocationCallbacks, &m_TransferCmdPool), VK_SUCCESS, "Failed to create transfer command pool.");
		if (m_TransferCmdPool == VK_NULL_HANDLE)
			return false;
		AURORA_VK_ATTACH_DEBUG_NAME(m_Device, VK_OBJECT_TYPE_COMMAND_POOL, (uint64_t)m_TransferCmdPool, "TransferCommandPool");

		SubmitToMainDeletionQueue([this]()
			{
				vkDestroyCommandPool(m_Device, m_TransferCmdPool, m_AllocationCallbacks);
				m_TransferCmdPool = VK_NULL_HANDLE;
			});

		VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		allocInfo.pNext = nullptr;
		allocInfo.commandPool = m_TransferCmdPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;
		AURORA_VK_CHECK(vkAllocateCommandBuffers(m_Device, &allocInfo, &m_TransferCmdBuffer), VK_SUCCESS, "Failed to allocate transfer command buffer for immediate submit.");
		if (m_TransferCmdBuffer == VK_NULL_HANDLE)
			return false;
		AURORA_VK_ATTACH_DEBUG_NAME(m_Device, VK_OBJECT_TYPE_COMMAND_BUFFER, (uint64_t)m_TransferCmdBuffer, "TransferCommandBuffer");

		VkSemaphoreTypeCreateInfo timelineCreateInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO };
		timelineCreateInfo.pNext = nullptr;
		timelineCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
		timelineCreateInfo.initialValue = 0;

		VkSemaphoreCreateInfo semaInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		semaInfo.pNext = &timelineCreateInfo;
		semaInfo.flags = 0;

		AURORA_VK_CHECK(vkCreateSemaphore(m_Device, &semaInfo, m_AllocationCallbacks, &m_TransferSubmitSemaphore.Semaphore), VK_SUCCESS, "Failed to create transfer semaphore.");
		if (m_TransferSubmitSemaphore.Semaphore == VK_NULL_HANDLE)
			return false;
		AURORA_VK_ATTACH_DEBUG_NAME(m_Device, VK_OBJECT_TYPE_SEMAPHORE, (uint64_t)m_TransferSubmitSemaphore.Semaphore, "TransferSubmitSemaphore");

		SubmitToMainDeletionQueue([this]()
			{
				vkDestroySemaphore(m_Device, m_TransferSubmitSemaphore.Semaphore, m_AllocationCallbacks);
				m_TransferSubmitSemaphore = {};
			});

		// ===== Graphics queue structures =====
		poolInfo.queueFamilyIndex = Helper::FindQueueFamilies(m_PhysicalDevice, m_Surface).Graphics;
		AURORA_VK_CHECK(vkCreateCommandPool(m_Device, &poolInfo, m_AllocationCallbacks, &m_GraphicsTransferCmdPool), VK_SUCCESS, "Failed to create graphics transfer command pool.");
		if (m_GraphicsTransferCmdPool == VK_NULL_HANDLE)
			return false;
		AURORA_VK_ATTACH_DEBUG_NAME(m_Device, VK_OBJECT_TYPE_COMMAND_POOL, (uint64_t)m_TransferCmdPool, "GraphicsTransferCommandPool");

		SubmitToMainDeletionQueue([this]()
			{
				vkDestroyCommandPool(m_Device, m_GraphicsTransferCmdPool, m_AllocationCallbacks);
				m_GraphicsTransferCmdPool = VK_NULL_HANDLE;
			});

		allocInfo.commandPool = m_GraphicsTransferCmdPool;
		AURORA_VK_CHECK(vkAllocateCommandBuffers(m_Device, &allocInfo, &m_GraphicsTransferCmdBuffer), VK_SUCCESS, "Failed to allocate graphics command buffer for immediate submit.");
		if (m_GraphicsTransferCmdBuffer == VK_NULL_HANDLE)
			return false;
		AURORA_VK_ATTACH_DEBUG_NAME(m_Device, VK_OBJECT_TYPE_COMMAND_BUFFER, (uint64_t)m_GraphicsTransferCmdBuffer, "GraphicsOwnershipCommandBuffer");

		AURORA_VK_CHECK(vkCreateSemaphore(m_Device, &semaInfo, m_AllocationCallbacks, &m_GraphicsSubmitSemaphore.Semaphore), VK_SUCCESS, "Failed to create transfer semaphore.");
		if (m_GraphicsSubmitSemaphore.Semaphore == VK_NULL_HANDLE)
			return false;
		AURORA_VK_ATTACH_DEBUG_NAME(m_Device, VK_OBJECT_TYPE_SEMAPHORE, (uint64_t)m_GraphicsSubmitSemaphore.Semaphore, "GraphicsSubmitSemaphore");

		SubmitToMainDeletionQueue([this]()
			{
				vkDestroySemaphore(m_Device, m_GraphicsSubmitSemaphore.Semaphore, m_AllocationCallbacks);
				m_GraphicsSubmitSemaphore = {};
			});

		// ===== Compute queue structures =====
		poolInfo.queueFamilyIndex = Helper::FindQueueFamilies(m_PhysicalDevice, m_Surface).Compute;
		AURORA_VK_CHECK(vkCreateCommandPool(m_Device, &poolInfo, m_AllocationCallbacks, &m_ComputeTransferCmdPool), VK_SUCCESS, "Failed to create compute transfer command pool.");
		if (m_ComputeTransferCmdPool == VK_NULL_HANDLE)
			return false;
		AURORA_VK_ATTACH_DEBUG_NAME(m_Device, VK_OBJECT_TYPE_COMMAND_POOL, (uint64_t)m_ComputeTransferCmdPool, "ComputeTransferCommandPool");

		SubmitToMainDeletionQueue([this]()
			{
				vkDestroyCommandPool(m_Device, m_ComputeTransferCmdPool, m_AllocationCallbacks);
				m_ComputeTransferCmdPool = VK_NULL_HANDLE;
			});

		allocInfo.commandPool = m_ComputeTransferCmdPool;
		AURORA_VK_CHECK(vkAllocateCommandBuffers(m_Device, &allocInfo, &m_ComputeTransferCmdBuffer), VK_SUCCESS, "Failed to allocate compute command buffer for immediate submit.");
		if (m_ComputeTransferCmdBuffer == VK_NULL_HANDLE)
			return false;
		AURORA_VK_ATTACH_DEBUG_NAME(m_Device, VK_OBJECT_TYPE_COMMAND_BUFFER, (uint64_t)m_ComputeTransferCmdBuffer, "ComputeOwnershipCommandBuffer");

		AURORA_VK_CHECK(vkCreateSemaphore(m_Device, &semaInfo, m_AllocationCallbacks, &m_ComputeSubmitSemaphore.Semaphore), VK_SUCCESS, "Failed to create compute submit semaphore.");
		if (m_ComputeSubmitSemaphore.Semaphore == VK_NULL_HANDLE)
			return false;
		AURORA_VK_ATTACH_DEBUG_NAME(m_Device, VK_OBJECT_TYPE_SEMAPHORE, (uint64_t)m_ComputeSubmitSemaphore.Semaphore, "ComputeSubmitSemaphore");

		SubmitToMainDeletionQueue([this]()
			{
				vkDestroySemaphore(m_Device, m_ComputeSubmitSemaphore.Semaphore, m_AllocationCallbacks);
				m_ComputeSubmitSemaphore = {};
			});

		return true;
	}

	bool VulkanContext::CreateFramesInFlight(uint8_t framesInFlight)
	{
		PROFILE_FUNCTION;

		VkCommandPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		poolInfo.pNext = nullptr;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = Helper::FindQueueFamilies(m_PhysicalDevice, m_Surface).Graphics;

		VkCommandBufferAllocateInfo cmdAllocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		cmdAllocInfo.pNext = nullptr;
		cmdAllocInfo.commandBufferCount = 1;
		cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;		

		VkFenceCreateInfo fenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		fenceInfo.pNext = nullptr;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		m_FramesInFlight.resize(framesInFlight);
		uint8_t i = 0;
		for (auto& fif : m_FramesInFlight)
		{
			fif.FrameIndex = i;
			const std::string iString = std::to_string(i);
			// ===== Frame command pool =====
			{
				const std::string poolName = "Frame_commandPool_" + iString;
				AURORA_VK_CHECK(vkCreateCommandPool(m_Device, &poolInfo, m_AllocationCallbacks, &fif.CommandPool), VK_SUCCESS, "Failed to create frame command pool.");
				if (fif.CommandPool == VK_NULL_HANDLE)
					return false;
				AURORA_VK_ATTACH_DEBUG_NAME(m_Device, VK_OBJECT_TYPE_COMMAND_POOL, (uint64_t)fif.CommandPool, poolName.c_str());

				SubmitToMainDeletionQueue([this, &pool = fif.CommandPool]()
					{
						vkDestroyCommandPool(m_Device, pool, m_AllocationCallbacks);
						pool = VK_NULL_HANDLE;
					});
			}

			// ===== Frame command buffer =====
			{
				cmdAllocInfo.commandPool = fif.CommandPool;
				const std::string cmdName = "Frame_commandBuffer_" + iString;
				AURORA_VK_CHECK(vkAllocateCommandBuffers(m_Device, &cmdAllocInfo, &fif.CommandBuffer), VK_SUCCESS, "Failed to allocate swapchain command buffer.");
				if (fif.CommandBuffer == VK_NULL_HANDLE)
					return false;
				AURORA_VK_ATTACH_DEBUG_NAME(m_Device, VK_OBJECT_TYPE_COMMAND_BUFFER, (uint64_t)fif.CommandBuffer, cmdName.c_str());
			}			

			// ===== Frame fence =====
			{
				const std::string inFlightName = "Frame_Fence_" + iString;
				AURORA_VK_CHECK(vkCreateFence(m_Device, &fenceInfo, m_AllocationCallbacks, &fif.InFlightFence), VK_SUCCESS, "Failed to create in-flight fence.");
				if (fif.InFlightFence == VK_NULL_HANDLE)
					return false;
				AURORA_VK_ATTACH_DEBUG_NAME(m_Device, VK_OBJECT_TYPE_FENCE, (uint64_t)fif.InFlightFence, inFlightName.c_str());

				SubmitToMainDeletionQueue([this, &fence = fif.InFlightFence]()
					{
						vkDestroyFence(m_Device, fence, m_AllocationCallbacks);
						fence = VK_NULL_HANDLE;
					});
			}
			i++;
		}
		return true;
	}

	bool VulkanContext::CreateSwapchain(const InitializationSpecification::SurfaceSpecification& surfaceSpecs)
	{
		PROFILE_FUNCTION;

		SwapchainSpecification swapchainSpecs{};
		swapchainSpecs.Device = m_Device;
		swapchainSpecs.AllocationCallbacks = m_AllocationCallbacks;
		swapchainSpecs.PhysicalDevice = m_PhysicalDevice;
		swapchainSpecs.Surface = m_Surface;
		swapchainSpecs.GraphicsQueue = m_QueueFamilies.Graphics;
		swapchainSpecs.PresentQueue = m_QueueFamilies.Present;
		swapchainSpecs.FramesInFlight = surfaceSpecs.FramesPerFlight;
		swapchainSpecs.VSync = surfaceSpecs.VSync;
		swapchainSpecs.InitialExtent = { surfaceSpecs.FramebufferWidth, surfaceSpecs.FramebufferHeight };
		swapchainSpecs.ClearColor = { surfaceSpecs.ClearColor.R, surfaceSpecs.ClearColor.G, surfaceSpecs.ClearColor.B, surfaceSpecs.ClearColor.A };
		m_Swapchain = VulkanSwapchain::Create(swapchainSpecs);

		if (m_Swapchain == nullptr)
		{
			AURORA_TRACE("Failed to create swapchain object.");
			return false;
		}

		m_Swapchain->Init();

		if (m_Swapchain->GetHandle() == VK_NULL_HANDLE)
		{
			AURORA_TRACE("Failed to initialize swapchain.");
			return false;
		}


		SubmitToMainDeletionQueue([this]()
		{
			m_Swapchain->Destroy();
			m_Swapchain = nullptr;
		});

		AURORA_TRACE("Created and initialized swapchain.");
		return true;
	}


	// == Frame management ==
	void VulkanContext::FlushFrameDeletionQueue(uint8_t frameIdx)
	{
		PROFILE_FUNCTION;

		if (frameIdx >= m_FramesInFlight.size())
		{
			AURORA_ERROR("Invalid frame index for deletion queue flush!");
			return;
		}

		m_FramesInFlight[frameIdx].DeletionQueue.Flush(m_Device);
	}

	void VulkanContext::AddPendingUpload(VkSemaphore semaphore, uint64_t signalValue, BufferHandle handle)
	{
		PROFILE_FUNCTION;

		PendingResourceUpload upload{};
		upload.SignalSemaphore = semaphore;
		upload.SignalValue = signalValue;
		upload.Handle = handle;
		m_PendingResourceUploads.push_back(upload);
	}

	void VulkanContext::PollPendingResourceUploads()
	{
		PROFILE_FUNCTION;


		for (size_t i = 0; i < m_PendingResourceUploads.size();)
		{
			const auto& res = m_PendingResourceUploads[i];
			uint64_t currentValue = 0;
			AURORA_VK_CHECK(vkGetSemaphoreCounterValue(m_Device, res.SignalSemaphore, &currentValue), VK_SUCCESS, "Failed to get semaphore counter value.");
			if (currentValue < res.SignalValue)
			{
				++i;
				continue;
			}

			if (VulkanBufferData* data = GetResourceManager()->GetBufferData(res.Handle))
			{
				data->IsReady = true;
				AURORA_INFO("Resource upload completed for buffer handle {}.", (uint16_t)res.Handle);
			}
			m_PendingResourceUploads[i] = m_PendingResourceUploads.back();
			m_PendingResourceUploads.pop_back();
		}
	}

	void VulkanContext::IncrementFramesInFlightIdx()
	{
		PROFILE_FUNCTION;

		m_RendererStatistics.FramesInFlightIdx = (m_RendererStatistics.FramesInFlightIdx + 1) % m_Specification.SurfaceSpecs.FramesPerFlight;
		m_RendererStatistics.TotalAttemptedFrames++;
	}

	// == Submissions == TODO: to be refactored
	void VulkanContext::AddDeferredBufferCopySubmissionOps(const std::vector<VulkanBufferCopyOp>& ops, bool forceNow /*= false*/)
	{
		PROFILE_FUNCTION;

		if(!forceNow)
			m_DeferredBufferCopySubmissionOps.insert(m_DeferredBufferCopySubmissionOps.end(), ops.begin(), ops.end());
		else
			for (const VulkanBufferCopyOp& op : ops)
				HandleBufferCopySubmissionOp(op);
	}

	void VulkanContext::HandleBufferCopySubmissionOp(const VulkanBufferCopyOp& op)
	{
		PROFILE_FUNCTION;

		Ref<VulkanResourceManager> res = GetResourceManager();

		VulkanBufferData* srcData = res->GetBufferData(op.Src);
		if (!srcData)
		{
			AURORA_ERROR("Invalid src buffer handle!");
			return;
		}
		VulkanBufferData* dstData = res->GetBufferData(op.Dst);
		if (!dstData)
		{
			AURORA_ERROR("Invalid dst buffer handle");
			return;
		}

		if (dstData->Size < srcData->Size)
		{
			AURORA_ERROR("Dst data too small for src");
			return;
		}

		//TODO: decide which queue should handle this
		QueueOwner copyQueue = QueueOwner::TRANSFER;
		uint32_t copyQueueIndex = GetQueueFamilyIndexFromOwner(copyQueue);
		std::vector<TimelineSemaphore> additionalWaits;
		//We need to realase them on their current queue if they are currently in an owned state (current != unknown) and if current differs from copyQueue
		if (srcData->CurrentOwner != QueueOwner::UNKNOWN && srcData->CurrentOwner != copyQueue)
		{
			uint32_t srcQueueIndex = GetQueueFamilyIndexFromOwner(srcData->CurrentOwner);
			SubmitSpecifications submitSpecs{ srcData->CurrentOwner };
			//6.1 transfer src ownership - release
			ImmediateSubmit([
				this,
				buffer = srcData->Buffer,
				offset = srcData->Offset,
				size = srcData->Size,
				srcQIndex = srcQueueIndex,
				dstQIndex = copyQueueIndex,
				lastUsage = srcData->Usage,
				queue = srcData->CurrentOwner
			](VkCommandBuffer cmd) {
				//Release barrier
				VkBufferMemoryBarrier2 barrier{ VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2 };
				barrier.pNext = nullptr;
				barrier.buffer = buffer;
				barrier.offset = offset;
				barrier.size = size;
				barrier.srcQueueFamilyIndex = srcQIndex;
				barrier.dstQueueFamilyIndex = dstQIndex;

				barrier.srcStageMask = GetStageFromBufferUsage(lastUsage, queue);
				barrier.srcAccessMask = GetAccessFromBufferUsage(lastUsage);
				barrier.dstStageMask = GetStageFromBufferUsage(BufferUsageFlags::TRANSFER_SRC);
				barrier.dstAccessMask = 0; // always				

				//TODO: refactor this out by first collecting all barriers + batching them together 
				VkDependencyInfo dependency{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
				dependency.pNext = nullptr;
				dependency.bufferMemoryBarrierCount = 1;
				dependency.pBufferMemoryBarriers = &barrier;
				dependency.dependencyFlags = VK_DEPENDENCY_QUEUE_FAMILY_OWNERSHIP_TRANSFER_USE_ALL_STAGES_BIT_KHR;
				vkCmdPipelineBarrier2(cmd, &dependency);
				//release barrier
			}, submitSpecs);
			srcData->LastOwner = srcData->CurrentOwner;
			srcData->CurrentOwner = QueueOwner::UNKNOWN;
			srcData->NextOwner = copyQueue;
			additionalWaits.push_back(GetSubmissionSemaFromQueueOwner(srcData->LastOwner));
		}

		//we cant handle both resources in one submit because they might be used in different pipeline stages, as well as different queues (src.CurrentOwner != dst.CurrentOwner) and we need to release them on their current queue
		if (dstData->CurrentOwner != QueueOwner::UNKNOWN && dstData->CurrentOwner != copyQueue)
		{
			uint32_t dstQueueIndex = GetQueueFamilyIndexFromOwner(dstData->CurrentOwner);
			SubmitSpecifications submitSpecs{ dstData->CurrentOwner };
			//6.1 transfer dst ownership - release
			ImmediateSubmit([
				this,
				buffer = dstData->Buffer,
				offset = dstData->Offset,
				size = dstData->Size,
				srcQIndex = dstQueueIndex,
				dstQIndex = copyQueueIndex,
				lastUsage = dstData->Usage,
				queue = dstData->CurrentOwner
			](VkCommandBuffer cmd) {
				//release barrier
				VkBufferMemoryBarrier2 barrier{ VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2 };
				barrier.pNext = nullptr;
				barrier.buffer = buffer;
				barrier.offset = offset;
				barrier.size = size;
				barrier.srcQueueFamilyIndex = srcQIndex;
				barrier.dstQueueFamilyIndex = dstQIndex;

				barrier.srcStageMask = GetStageFromBufferUsage(lastUsage);
				barrier.srcAccessMask = GetAccessFromBufferUsage(lastUsage);
				barrier.dstStageMask = GetStageFromBufferUsage(BufferUsageFlags::TRANSFER_DST);
				barrier.dstAccessMask = 0; // always

				VkDependencyInfo dependency{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
				dependency.pNext = nullptr;
				dependency.bufferMemoryBarrierCount = 1;
				dependency.pBufferMemoryBarriers = &barrier;
				dependency.dependencyFlags = VK_DEPENDENCY_QUEUE_FAMILY_OWNERSHIP_TRANSFER_USE_ALL_STAGES_BIT_KHR;

				vkCmdPipelineBarrier2(cmd, &dependency);
			}, submitSpecs);

			dstData->LastOwner = dstData->CurrentOwner;
			dstData->CurrentOwner = QueueOwner::UNKNOWN;
			dstData->NextOwner = copyQueue;
			additionalWaits.push_back(GetSubmissionSemaFromQueueOwner(dstData->LastOwner));
		}

		//No sure if we need to check if current is UNKNOWN, because we set current owner to UNKNOWN after releasing, so technically we need to check if last was unknown as well
		bool srcNeedsAcquire = srcData->CurrentOwner != copyQueue && srcData->LastOwner != QueueOwner::UNKNOWN;
		bool dstNeedsAcquire = dstData->CurrentOwner != copyQueue && dstData->LastOwner != QueueOwner::UNKNOWN;
		bool srcNeedsRelease = !op.DestroySrc && srcData->NextOwner != copyQueue;
		bool dstNeedsRelease = op.NextDstOwner != copyQueue;
		uint32_t srcCurQueueIndex = GetQueueFamilyIndexFromOwner(srcData->LastOwner);
		uint32_t dstCurQueueIndex = GetQueueFamilyIndexFromOwner(dstData->LastOwner);
		uint32_t dstTarQueueIndex = GetQueueFamilyIndexFromOwner(op.NextDstOwner);
		
		SubmitSpecifications submitSpecs{ copyQueue };
		submitSpecs.AdditionalWaitSemaphores = additionalWaits;
		ImmediateSubmit([
			this,
			srcNeedsAcquire = srcNeedsAcquire,
			dstNeedsAcquire = dstNeedsAcquire,
			srcBuffer = srcData->Buffer,
			dstBuffer = dstData->Buffer,
			srcOffset = srcData->Offset,
			dstOffset = dstData->Offset,
			srcNeedsRelease = srcNeedsRelease,
			dstNeedsRelease = dstNeedsRelease,
			size = srcData->Size,
			srcCurQueueIndex = srcCurQueueIndex,
			dstCurQueueIndex = dstCurQueueIndex,
			copyQueueIndex = copyQueueIndex,
			dstTarQueueIndex = dstTarQueueIndex,
			dstUsage = dstData->Usage
		](VkCommandBuffer cmd) {

			VkBufferMemoryBarrier2 acquireBarriers[2];
			uint32_t acquireBarrierCount = 0;
			if (srcNeedsAcquire)
				acquireBarriers[acquireBarrierCount++] = Creators::EmitAcquireBarrier(srcBuffer,
					srcOffset,
					size,
					srcCurQueueIndex,
					copyQueueIndex,
//TODO: usage will change depending on the queue that does the copy -> we will later implement a metrix (buffer size or queue workload) to decide which queue should do the copy,
// and then we will set the usage accordingly
					GetStageFromBufferUsage(BufferUsageFlags::TRANSFER_SRC),
					GetStageFromBufferUsage(BufferUsageFlags::TRANSFER_SRC),
					GetAccessFromBufferUsage(BufferUsageFlags::TRANSFER_SRC));

			if (dstNeedsAcquire)
				acquireBarriers[acquireBarrierCount++] = Creators::EmitAcquireBarrier(dstBuffer,
					dstOffset,
					size,
					dstCurQueueIndex,
					copyQueueIndex,
//TODO: usage will change depending on the queue that does the copy -> we will later implement a metrix (buffer size or queue workload) to decide which queue should do the copy,
// and then we will set the usage accordingly
					GetStageFromBufferUsage(BufferUsageFlags::TRANSFER_DST),
					GetStageFromBufferUsage(BufferUsageFlags::TRANSFER_DST),
					GetAccessFromBufferUsage(BufferUsageFlags::TRANSFER_DST));

			if (acquireBarrierCount > 0)
			{
				VkDependencyInfo dependency{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
				dependency.pNext = nullptr;
				dependency.bufferMemoryBarrierCount = acquireBarrierCount;
				dependency.pBufferMemoryBarriers = &acquireBarriers[0];
				dependency.dependencyFlags = VK_DEPENDENCY_QUEUE_FAMILY_OWNERSHIP_TRANSFER_USE_ALL_STAGES_BIT_KHR;

				vkCmdPipelineBarrier2(cmd, &dependency);
			}

			// Copy
			VkBufferCopy2 region{ VK_STRUCTURE_TYPE_BUFFER_COPY_2 };
			region.pNext = nullptr;
			region.srcOffset = srcOffset;
			region.dstOffset = dstOffset;
			region.size = size;

			VkCopyBufferInfo2 copy{ VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2 };
			copy.pNext = nullptr;
			copy.srcBuffer = srcBuffer;
			copy.dstBuffer = dstBuffer;
			copy.regionCount = 1;
			copy.pRegions = &region;

			vkCmdCopyBuffer2(cmd, &copy);

			if (dstNeedsRelease)
			{
				VkBufferMemoryBarrier2 releaseBarrier = Creators::EmitReleaseBarrier(dstBuffer,
					dstOffset,
					size,
					copyQueueIndex,
					dstTarQueueIndex,
//TODO: usage will change depending on the queue that does the copy -> we will later implement a metrix (buffer size or queue workload) to decide which queue should do the copy,
// and then we will set the usage accordingly
					GetStageFromBufferUsage(BufferUsageFlags::TRANSFER_DST),
					GetAccessFromBufferUsage(BufferUsageFlags::TRANSFER_DST),
					GetStageFromBufferUsage(BufferUsageFlags::TRANSFER_DST));
				VkDependencyInfo dependency{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
				dependency.pNext = nullptr;
				dependency.bufferMemoryBarrierCount = 1;
				dependency.pBufferMemoryBarriers = &releaseBarrier;
				dependency.dependencyFlags = VK_DEPENDENCY_QUEUE_FAMILY_OWNERSHIP_TRANSFER_USE_ALL_STAGES_BIT_KHR;
				vkCmdPipelineBarrier2(cmd, &dependency);
			}
		}, submitSpecs);
				

		srcData->CurrentOwner = copyQueue;
		srcData->NextOwner = op.NextDstOwner;


		dstData->LastOwner = copyQueue;
		dstData->CurrentOwner = QueueOwner::UNKNOWN;
		dstData->NextOwner = op.NextDstOwner;

		TimelineSemaphore uploadSignalSema = GetSubmissionSemaFromQueueOwner(dstData->LastOwner);
		AddPendingUpload(uploadSignalSema.Semaphore, uploadSignalSema.Value, op.Dst);

		if (dstNeedsRelease)
		{
			additionalWaits.clear();
			additionalWaits.push_back(uploadSignalSema);
			uint32_t srcQueueIndex = GetQueueFamilyIndexFromOwner(dstData->LastOwner);
			uint32_t dstQueueIndex = GetQueueFamilyIndexFromOwner(dstData->NextOwner);
			SubmitSpecifications submitSpecs{ dstData->NextOwner, additionalWaits };
			ImmediateSubmit([
				this,
				buffer = dstData->Buffer,
				offset = dstData->Offset,
				size = dstData->Size,
				srcQIndex = srcQueueIndex,
				dstQIndex = dstQueueIndex,
				lastUsage = dstData->Usage,
				queue = op.NextDstOwner
			](VkCommandBuffer cmd) {
				//acquire barrier
				VkBufferMemoryBarrier2 acquireBarrier = Creators::EmitAcquireBarrier(buffer,
					offset,
					size,
					srcQIndex,
					dstQIndex,
					GetStageFromBufferUsage(BufferUsageFlags::TRANSFER_SRC),
					GetStageFromBufferUsage(lastUsage),
					GetAccessFromBufferUsage(lastUsage));

				VkDependencyInfo dependency{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
				dependency.pNext = nullptr;
				dependency.bufferMemoryBarrierCount = 1;
				dependency.pBufferMemoryBarriers = &acquireBarrier;
				dependency.dependencyFlags = VK_DEPENDENCY_QUEUE_FAMILY_OWNERSHIP_TRANSFER_USE_ALL_STAGES_BIT_KHR;

				vkCmdPipelineBarrier2(cmd, &dependency);
			}, submitSpecs);
		}

		dstData->LastOwner = copyQueue;
		dstData->CurrentOwner = op.NextDstOwner;
		dstData->NextOwner = QueueOwner::UNKNOWN;				

		if (op.DestroySrc)
			res->DestroyBuffer(op.Src);
	}


	void VulkanContext::FlushDeferredSubmissionOps()
	{
		PROFILE_FUNCTION;

		for (const VulkanBufferCopyOp& op : m_DeferredBufferCopySubmissionOps)
		{
			switch (op.Type)
			{
				case SubmissionOpType::COPY_BUFFER:
				{
					HandleBufferCopySubmissionOp(op);
					break;
				}
				default:
				{
					AURORA_ERROR("Unknown submission op type!");
					return;
				}
			}
		}

		//TODO: for now we just clear the ops, but we might want to keep them around for debugging or replaying later
		m_DeferredBufferCopySubmissionOps.clear();
	}	

	void VulkanContext::CopyBufferToBuffer(BufferHandle src, BufferHandle dst, bool forceNow/* = false*/, bool destroySrc/* =true*/)
	{
		PROFILE_FUNCTION;

		if (src == dst)
		{
			AURORA_ERROR("Src handle equals dst handle");
			return;
		}

		Ref<VulkanResourceManager> res = GetResourceManager();
		VulkanBufferData* srcData = res->GetBufferData(src);
		if (!srcData)
		{
			AURORA_ERROR("Invalid src buffer handle!");
			return;
		}
		VulkanBufferData* dstData = res->GetBufferData(dst);
		if (!dstData)
		{
			AURORA_ERROR("Invalid dst buffer handle");
			return;
		}

		if (dstData->Size < srcData->Size)
		{
			AURORA_ERROR("Dst data too small for src");
			return;
		}

		VulkanSubmissionScheduler(CreateRefFromThis<VulkanContext>())
			.CopyBufferToBuffer(src, dst, destroySrc, QueueOwner::GRAPHICS)
			.ScheduleSubmissions(forceNow);
	}


	void VulkanContext::ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& recordFunc, const SubmitSpecifications& specs)
	{
		PROFILE_FUNCTION;

		//AURORA_VK_CHECK(vkWaitForFences(m_Device, 1, &m_TransferFence, VK_TRUE, UINT64_MAX), VK_SUCCESS, "Failed to wait for transfer fence.");
		//AURORA_VK_CHECK(vkResetFences(m_Device, 1, &m_TransferFence), VK_SUCCESS, "Failed to reset transfer fence.");

		VkCommandBuffer cmd = GetCommandBufferFromQueueOwner(specs.Queue);
		TimelineSemaphore& semaphore = GetSubmissionSemaFromQueueOwner(specs.Queue);
		std::vector<TimelineSemaphore> waits = specs.AdditionalWaitSemaphores;
		waits.push_back(semaphore);

		std::vector<VkSemaphoreSubmitInfo> waitSemaphoreInfos;
		for (const TimelineSemaphore& semaphore : waits)
		{
			VkSemaphoreSubmitInfo waitSemaphoreInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO };
			waitSemaphoreInfo.pNext = nullptr;
			waitSemaphoreInfo.semaphore = semaphore.Semaphore;
			waitSemaphoreInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
			waitSemaphoreInfo.deviceIndex = 0;
			waitSemaphoreInfo.value = semaphore.Value;
			waitSemaphoreInfos.push_back(waitSemaphoreInfo);
		}

		VkSemaphoreSubmitInfo signalSemaphoreInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO };
		signalSemaphoreInfo.pNext = nullptr;
		signalSemaphoreInfo.semaphore = semaphore.Semaphore;
		signalSemaphoreInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		signalSemaphoreInfo.deviceIndex = 0;
		signalSemaphoreInfo.value = ++semaphore.Value;

		VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		beginInfo.pNext = nullptr;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		beginInfo.pInheritanceInfo = nullptr;

		AURORA_VK_CHECK(vkBeginCommandBuffer(cmd, &beginInfo), VK_SUCCESS, "Failed to begin command buffer for immediate submit.");
		recordFunc(cmd);
		AURORA_VK_CHECK(vkEndCommandBuffer(cmd), VK_SUCCESS, "Failed to end command buffer for immediate submit.");

		VkCommandBufferSubmitInfo cmdBufferSubmitInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO };
		cmdBufferSubmitInfo.pNext = nullptr;
		cmdBufferSubmitInfo.commandBuffer = cmd;
		cmdBufferSubmitInfo.deviceMask = 0;

		VkSubmitInfo2 submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO_2 };
		submitInfo.pNext = nullptr;
		submitInfo.flags = 0;
		submitInfo.commandBufferInfoCount = 1;
		submitInfo.pCommandBufferInfos = &cmdBufferSubmitInfo;
		submitInfo.waitSemaphoreInfoCount = static_cast<uint32_t>(waitSemaphoreInfos.size());
		submitInfo.pWaitSemaphoreInfos = waitSemaphoreInfos.data();
		submitInfo.signalSemaphoreInfoCount = 1;
		submitInfo.pSignalSemaphoreInfos = &signalSemaphoreInfo;

		VkQueue vkQueue = GetQueueFromOwner(specs.Queue);
		if (vkQueue == VK_NULL_HANDLE)
		{
			AURORA_ERROR("Failed to get queue for immediate submit.");
			return;
		}

		AURORA_VK_CHECK(vkQueueSubmit2(vkQueue, 1, &submitInfo, nullptr), VK_SUCCESS, "Failed to submit command buffer for immediate submit.");
	}

	TimelineSemaphore VulkanContext::GetQueueSemaphoreSnapshot(QueueOwner owner)
	{
		if (owner == QueueOwner::UNKNOWN)
			return {};
		return GetSubmissionSemaFromQueueOwner(owner);
	}

	uint32_t VulkanContext::GetQueueFamilyIndexFromOwner(QueueOwner owner) const
	{
		auto it = m_QueueOwnerIndices.find(owner);
		if (it != m_QueueOwnerIndices.end())
			return it->second;

		AURORA_ERROR("Queue family index for owner {} not found.", QueueOwnerToString(owner));
		return VK_QUEUE_FAMILY_IGNORED;
	}
	VkQueue VulkanContext::GetQueueFromOwner(QueueOwner owner) const
	{
		auto it = m_QueueOwnerQueues.find(owner);
		if (it != m_QueueOwnerQueues.end())
			return it->second;

		AURORA_ERROR("Queue for owner {} not found.", QueueOwnerToString(owner));
		return VK_NULL_HANDLE;
	}
	TimelineSemaphore& VulkanContext::GetSubmissionSemaFromQueueOwner(QueueOwner owner)
	{
		PROFILE_FUNCTION;

		switch (owner)
		{
			case QueueOwner::GRAPHICS:
				return m_GraphicsSubmitSemaphore;
				break;
			case QueueOwner::TRANSFER:
				return m_TransferSubmitSemaphore;
				break;
			case QueueOwner::COMPUTE:
				return m_ComputeSubmitSemaphore;
				break;
			default:
				AURORA_ERROR("Invalid queue owner for TimelineSemaphore. Using Graphics semaphore (default).");
				AURORA_ASSERT(false, "Invalid queue owner for TimelineSemaphore selection.");
				return m_GraphicsSubmitSemaphore;
		}
	}
	VkCommandBuffer VulkanContext::GetCommandBufferFromQueueOwner(QueueOwner owner)
	{
		PROFILE_FUNCTION;
		VkCommandBuffer cmd = VK_NULL_HANDLE;
		switch (owner)
		{
			case QueueOwner::GRAPHICS:
				cmd = m_GraphicsTransferCmdBuffer;
				break;
			case QueueOwner::TRANSFER:
				cmd = m_TransferCmdBuffer;
				break;
			case QueueOwner::COMPUTE:
				cmd = m_ComputeTransferCmdBuffer;
				break;
			default:
				AURORA_ERROR("Invalid queue owner for CommandBuffer. Using Graphics command buffer (default).");
				AURORA_ASSERT(false, "Invalid queue owner for CommandBuffer selection.");
				return m_GraphicsTransferCmdBuffer;
		}
		return cmd;
	}

	bool VulkanContext::CheckTimelineSemaphore(VkSemaphore sema, uint64_t targetValue) const
	{
		PROFILE_FUNCTION;

		uint64_t currentValue;
		AURORA_VK_CHECK(vkGetSemaphoreCounterValue(m_Device, sema, &currentValue), VK_SUCCESS, "Failed to get semaphore counter value!");
		return currentValue >= targetValue;
	}


	// ========== Privat helper ==========

	int VulkanContext::EvaluatePhysicalDevice(VkPhysicalDevice phDevice, const DeviceRequirements& deviceRequirements) const
	{
		PROFILE_FUNCTION;

		int score = 0;

		QueueFamilyIndices indices = Helper::FindQueueFamilies(phDevice, m_Surface);
		if (!indices.IsComplete())
			return 0;

		if (!CheckRequiredDeviceExtensionSupport(phDevice, deviceRequirements))
			return 0;

		auto swapchainSupportDetails = Helper::GetSwapSupportDetails(phDevice, m_Surface);
		if (!swapchainSupportDetails.Formats.empty() && !swapchainSupportDetails.PresentModes.empty() && swapchainSupportDetails.Capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
		{
			//TODO: handle more detailed selection here (look for specific format etc)
			score += 10; //magic number
		}
		else
			return 0;

		if (indices.HasDedicatedCompute)
			score += 100; //magic number
		if (indices.HasDedicatedTransfer)
			score += 200; //magic number

		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(phDevice, &deviceProperties);

		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(phDevice, &deviceFeatures);
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			score += 1000; //magic number

		return score;
	}

	bool VulkanContext::CheckRequiredDeviceExtensionSupport(VkPhysicalDevice phDevice, const DeviceRequirements& deviceRequirements) const
	{
		PROFILE_FUNCTION;

		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(phDevice, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(phDevice, nullptr, &extensionCount, availableExtensions.data());

		auto requiredExtensionsList = GetRequiredDeviceExtensions(deviceRequirements);
		std::set<std::string> requiredExtensions(requiredExtensionsList.begin(), requiredExtensionsList.end());

		for (const auto& extension : availableExtensions)
		{
			if (requiredExtensions.find(extension.extensionName) == requiredExtensions.end())
				continue;

			AURORA_INFO("Found required device extension {}", extension.extensionName);
			requiredExtensions.erase(extension.extensionName);

			if (requiredExtensions.empty())
				return true;
		}
		for (const auto& extension : requiredExtensions)
		{
			AURORA_ERROR("Required extension {} not supported by device.", extension);
		}

		return false;
	}

	const std::vector<const char*> VulkanContext::GetRequiredDeviceExtensions(const DeviceRequirements& deviceRequirements) const
	{
		//TODO: Write parsing between deviceRequirements and the actual extensions

		return { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_MAINTENANCE_8_EXTENSION_NAME };
	}

	bool VulkanContext::CheckRequiredLayerSupport(const std::vector<const char*>& requiredLayers) const
	{
		PROFILE_FUNCTION;

		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		bool allLayerFound = true;
		for (const auto& layer : requiredLayers)
		{
			bool layerFound = false;
			for (const auto& layerProps : availableLayers)
			{
				if (strcmp(layer, layerProps.layerName) == 0)
				{
					layerFound = true;
					AURORA_INFO("Instance layer {} found.", layer);
					break;
				}
			}
			if (!layerFound)
			{
				allLayerFound = false;
				AURORA_ERROR("Requested instance layer {} not available!", layer);
			}
		}
		return allLayerFound;
	}

	std::vector<const char*> VulkanContext::GetRequiredInstanceExtensions(WSIPlatformType wsi) const
	{
		PROFILE_FUNCTION;

		std::vector<const char*> extensions;
		if (wsi == WSIPlatformType::SURFACE_PLATFORM_GLFW)
		{
			uint32_t glfwExtensionCount;
			const char** glfwExtensions;
			glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
			extensions.insert(extensions.end(), glfwExtensions, (glfwExtensions + glfwExtensionCount));
		}
		return extensions;
	}

	bool VulkanContext::CheckRequiredInstanceExtensionsSupport(const std::vector<const char*>& requiredExtensions) const
	{
		PROFILE_FUNCTION;

		uint32_t extensionCount;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

		bool foundAll = true;
		for (const auto& required : requiredExtensions)
		{
			bool foundExtension = false;
			for (const auto& available : availableExtensions)
			{
				if (strcmp(required, available.extensionName) == 0)
				{
					foundExtension = true;
					AURORA_INFO("Required instance extension {} found.", required);
					break;
				}
			}

			if (!foundExtension)
			{
				foundAll = false;
				AURORA_ERROR("Required instance extension {} not availale!", required);
			}
		}

		return foundAll;
	}

	//Global messenger for AFTER instance creation
	void VulkanContext::SetupDebugMessenger(VkInstance instance, bool enableInfoDebugLevel /*= false*/)
	{
		PROFILE_FUNCTION;

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		PopulateDebugMessengerCreateInfo(createInfo, enableInfoDebugLevel);

		AURORA_VK_CHECK(Debug::CreateDebugUtilsMessengerEXT(instance, &createInfo, m_AllocationCallbacks, &m_DebugMessenger), VK_SUCCESS, "Failed to create debug messenger.");

		SubmitToMainDeletionQueue([instance, this]()
			{
				Debug::DestroyDebugUtilsMessengerEXT(instance, m_DebugMessenger, m_AllocationCallbacks);
				m_DebugMessenger = VK_NULL_HANDLE;
			});
	}

	void VulkanContext::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo, bool allowInfoLevel /*= false*/)
	{
		PROFILE_FUNCTION;

		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;

		VkDebugUtilsMessageSeverityFlagsEXT severityFlags = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
#if defined(AURORA_DEBUG_MODE) && allowInfoLevel
		severityFlags |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
#endif
		createInfo.messageSeverity = severityFlags;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = Debug::VulkanDebugCallback;
	}

}


