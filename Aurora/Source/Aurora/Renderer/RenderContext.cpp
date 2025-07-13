#include "Aurora/Core/Core.h"
#include "Aurora/Renderer/RenderContext.h"

#include "Aurora/Renderer/VulkanHelper.h"
#include "Aurora/Renderer/DataStructs/QueueFamilies.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <map>
#include <set>
#include <string>

namespace Aurora::VK {

	namespace Utils {

		static constexpr std::string DeviceTypeToString(VkPhysicalDeviceType type)
		{
			switch (type)
			{
				case VK_PHYSICAL_DEVICE_TYPE_CPU: return "CPU";
				case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: return "Discrete GPU";
				case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return "Integrated GPU";
				case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: return "Virtual GPU";
				default: return "Other";
			}
		}

		static constexpr std::string ApiVersionToString(uint32_t version)
		{
			uint32_t major = VK_API_VERSION_MAJOR(version);
			uint32_t minor = VK_API_VERSION_MINOR(version);
			uint32_t variant = VK_API_VERSION_VARIANT(version);
			return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(variant);
		}
	}


	RenderContext::RenderContext(const RenderContextSpecification& specs)
		: m_Specification(specs)
	{
	}


	void RenderContext::Init()
	{
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

		if (!CreateGraphicsCmdPool())
		{
			AURORA_TRACE("Failed to create graphics command pool. VulkanContext could not be initialized.");
			return;
		}

		if (!CreateSwapchain(m_Specification.SurfaceSpecs))
		{
			AURORA_ERROR("Failed to create swapchain. VulkanContext could not be initialized.");
			return;
		}		

		//This initializes FIF, such that the very first rendered frame still is index 0;
		m_RendererState.FramesInFlightIdx = m_Specification.SurfaceSpecs.FramesPerFlight - 1;
		AURORA_INFO("Successfully initialized vulkan rendering context");
	}


	void RenderContext::Destroy()
	{
		vkDeviceWaitIdle(m_Device);

		m_Swapchain->Destroy();
		m_Swapchain = nullptr;

		vkDestroyCommandPool(m_Device, m_GraphicsCmdPool, nullptr);
		m_GraphicsCmdPool = VK_NULL_HANDLE;

		vmaDestroyAllocator(m_VmAllocator);
		m_VmAllocator = VK_NULL_HANDLE;

		vkDestroyDevice(m_Device, nullptr);
		m_Device = VK_NULL_HANDLE;

		vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
		m_Surface = VK_NULL_HANDLE;
				
		Debug::DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
		m_DebugMessenger = VK_NULL_HANDLE;
		
		vkDestroyInstance(m_Instance, nullptr);
		m_Instance = VK_NULL_HANDLE;

		AURORA_INFO("Destroyed all vulkan context objects.");
	}


	bool RenderContext::BeginFrame()
	{
		IncrementFramesInFlightIdx();
		AURORA_TRACE("Beginning frame {}", m_RendererState.FramesInFlightIdx);
		//Acquire next image available image from swapchain
		//pass relevant information to renderers
		if (!m_Swapchain->PrepareFrame(m_RendererState.FramesInFlightIdx))
			return false;

		//Todo: move into call "start recording"
		const FrameData* frame = m_Swapchain->GetCurrentFrameData();

		VkCommandBufferBeginInfo cmdInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		cmdInfo.pNext = nullptr;
		cmdInfo.pInheritanceInfo = nullptr;
		cmdInfo.flags = 0;

		AURORA_VK_CHECK(vkBeginCommandBuffer(frame->CommandBuffer, &cmdInfo), VK_SUCCESS, "Failed to begin command buffer (frame index: {}).", frame->FrameIndex);

		//TEMP:
		m_Swapchain->RecordFallbackSwapchainRenderPass();

		return true;
	}

	void RenderContext::EndFrame()
	{
		//Todo: move into call "end recording"
		//finalize command buffers
		//pass relevant information to swapchain (submit)
		const FrameData* frame = m_Swapchain->GetCurrentFrameData();
		AURORA_VK_CHECK(vkEndCommandBuffer(frame->CommandBuffer), VK_SUCCESS, "Failed to end command buffer (frame index: {}).", frame->FrameIndex);
	}

	void RenderContext::SwapFrame()
	{
		if (m_Swapchain->SwapImages())
			m_RendererState.m_TotalFinishedFrames++;
	}

	void RenderContext::Resize(uint32_t width, uint32_t height)
	{
		m_Swapchain->OnResize(width, height);
	}

	// ========== Object Creation ==========
	bool RenderContext::CreateInstance(
		const std::string& appName,
		const RenderContextSpecification::InstanceSpecification instanceSpecs, 
		RenderContextSpecification::ApplicationVersionNumber appVersion, 
		RenderContextSpecification::AuroraVersionNumber auroraVersion,
		WSIPlatformType wsi)
	{		

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
		appInfo.apiVersion = VK_API_VERSION_1_3;
		appInfo.pApplicationName = appName.c_str();
		appInfo.applicationVersion = VK_MAKE_VERSION(appVersion.Major, appVersion.Minor, appVersion.Patch);
		appInfo.engineVersion = VK_MAKE_VERSION(auroraVersion.Major, auroraVersion.Minor, auroraVersion.Patch);

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

		AURORA_VK_CHECK(vkCreateInstance(&instanceInfo, nullptr, &m_Instance), VK_SUCCESS, "Failed to create VkInstance.");
		AURORA_TRACE("Created VkInstance.");


#if AURORA_VK_VALIDATION_ENABLED
		if (useDebugUtils)
			SetupDebugMessenger(m_Instance);
#endif // AURORA_VK_VALIDATION


		AURORA_TRACE("Created vulkan instance.");
		return true;
	}


	bool RenderContext::CreateSurface(const RenderContextSpecification::SurfaceSpecification& surfaceSpecs)
	{
		switch (surfaceSpecs.WSI)
		{
			case WSIPlatformType::SURFACE_PLATFORM_GLFW:
			{
				AURORA_VK_CHECK(glfwCreateWindowSurface(m_Instance, (GLFWwindow*)surfaceSpecs.WindowHandle, nullptr, &m_Surface), VK_SUCCESS, "Failed to create GLFWWindow Surface.");
				break;
			}
			case WSIPlatformType::SURFACE_PLAFORM_NONE:				
			default:
			{
				AURORA_ERROR("WSI currently not supported!");
				return false;
			}
		}

		if (m_Surface == nullptr)
		{
			AURORA_CRITICAL("Failed to create a Vulkan Surface!");
			return false;
		}

		AURORA_TRACE("Created vulkan surface.");
		return true;
	}
	
	bool RenderContext::PickPhysicalDevice(const DeviceRequirements& deviceRequirements)
	{
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
		AURORA_INFO("Type			: {}", Utils::DeviceTypeToString(props.deviceType));
		AURORA_INFO("API Version	: {}", Utils::ApiVersionToString(props.apiVersion));
		AURORA_INFO("DeviceID		: {}", props.deviceID);
		AURORA_INFO("==========================================================\n");


		AURORA_TRACE("Picked physical device");
		return true;
	}

	bool RenderContext::CreateLogicalDevice(const DeviceRequirements& deviceRequirements)
	{
		QueueFamilyIndices indices = Helper::FindQueueFamilies(m_PhysicalDevice, m_Surface);
		size_t uniqueQueueFamilies = indices.UniqueFamilyIndices();
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
		for(const auto& index : uniqueQueueFamilyIndices)
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
		VkDeviceCreateInfo deviceInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
		deviceInfo.pNext = nullptr;
		deviceInfo.flags = 0;
		deviceInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtension.size());
		deviceInfo.ppEnabledExtensionNames = requiredExtension.data();
		deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
		deviceInfo.pQueueCreateInfos = queueInfos.data();
		deviceInfo.pEnabledFeatures = &features;
		AURORA_VK_CHECK(vkCreateDevice(m_PhysicalDevice, &deviceInfo, nullptr, &m_Device), VK_SUCCESS, "Failed to create device!");

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


		AURORA_TRACE("Created vulkan logical device and gathered queues");
		return true;
	}

	bool RenderContext::CreateVmAllocator()
	{
		VmaAllocatorCreateInfo alInfo{};
		alInfo.instance = m_Instance;
		alInfo.device = m_Device;
		alInfo.physicalDevice = m_PhysicalDevice;
		alInfo.vulkanApiVersion = VK_API_VERSION_1_3;
		alInfo.flags = 0;
		alInfo.pAllocationCallbacks = nullptr;
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
		return true;
	}

	bool RenderContext::CreateGraphicsCmdPool()
	{
		VkCommandPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		poolInfo.pNext = nullptr;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = Helper::FindQueueFamilies(m_PhysicalDevice, m_Surface).Graphics;
		
		AURORA_VK_CHECK(vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_GraphicsCmdPool), VK_SUCCESS, "Failed to create graphics command pool.");
		if (m_GraphicsCmdPool == VK_NULL_HANDLE)
			return false;
		AURORA_VK_ATTACH_DEBUG_NAME(m_Device, VK_OBJECT_TYPE_COMMAND_POOL, (uint64_t)m_GraphicsCmdPool, "GraphicsCommandPool");

		return true;
	}

	bool RenderContext::CreateSwapchain(const RenderContextSpecification::SurfaceSpecification& surfaceSpecs)
	{
		SwapchainSpecification swapchainSpecs{};
		swapchainSpecs.Device = m_Device;
		swapchainSpecs.PhysicalDevice = m_PhysicalDevice;
		swapchainSpecs.Surface = m_Surface;
		swapchainSpecs.GraphicsCmdPool = m_GraphicsCmdPool;
		swapchainSpecs.GraphicsQueue = m_QueueFamilies.Graphics;
		swapchainSpecs.PresentQueue = m_QueueFamilies.Present;
		swapchainSpecs.FramesInFlight = surfaceSpecs.FramesPerFlight;
		swapchainSpecs.VSync = surfaceSpecs.VSync;
		swapchainSpecs.InitialExtent = { surfaceSpecs.FramebufferWidth, surfaceSpecs.FramebufferHeight };
		swapchainSpecs.ClearColor = { surfaceSpecs.ClearColor.R, surfaceSpecs.ClearColor.G, surfaceSpecs.ClearColor.B, surfaceSpecs.ClearColor.A };
		m_Swapchain = CreateRef<Swapchain>(swapchainSpecs);

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
		
		
		AURORA_TRACE("Created and initialized swapchain.");
		return true;
	}	
	
	void RenderContext::IncrementFramesInFlightIdx()
	{
		m_RendererState.FramesInFlightIdx = (m_RendererState.FramesInFlightIdx + 1) % m_Specification.SurfaceSpecs.FramesPerFlight;
		m_RendererState.TotalAttemptedFrames++;
	}


	// ========== Privat helper ==========

	int RenderContext::EvaluatePhysicalDevice(VkPhysicalDevice phDevice, const DeviceRequirements& deviceRequirements) const
	{
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

	bool RenderContext::CheckRequiredDeviceExtensionSupport(VkPhysicalDevice phDevice, const DeviceRequirements& deviceRequirements) const
	{
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

	const std::vector<const char*> RenderContext::GetRequiredDeviceExtensions(const DeviceRequirements& deviceRequirements) const
	{
		//TODO: Write parsing between devideRequirements and the actual extensions

		return { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	}

	bool RenderContext::CheckRequiredLayerSupport(const std::vector<const char*>& requiredLayers) const
	{
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

	std::vector<const char*> RenderContext::GetRequiredInstanceExtensions(WSIPlatformType wsi) const
	{
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

	bool RenderContext::CheckRequiredInstanceExtensionsSupport(const std::vector<const char*>& requiredExtensions) const
	{
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
				if(strcmp(required, available.extensionName) == 0)
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
	void RenderContext::SetupDebugMessenger(VkInstance instance, bool enableInfoDebugLevel /*= false*/)
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		PopulateDebugMessengerCreateInfo(createInfo, enableInfoDebugLevel);

		AURORA_VK_CHECK(Debug::CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &m_DebugMessenger), VK_SUCCESS, "Failed to create debug messenger.");
	}

	void RenderContext::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo, bool allowInfoLevel /*= false*/)
	{
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


