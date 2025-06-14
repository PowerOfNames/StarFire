#include "Renderer/VulkanSpecifics/VulkanCore.h"
#include "Renderer/VulkanSpecifics/VulkanRenderContext.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <map>
#include <set>

namespace Aurora { namespace VK {

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


	VulkanRenderContext::VulkanRenderContext(const RenderContextSpecification& specs)
		: m_Specification(specs)
	{
	}


	void VulkanRenderContext::Init()
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
	}


	void VulkanRenderContext::Shutdown()
	{
		vkDestroyDevice(m_Device, nullptr);
		m_Device = nullptr;

		vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
		m_Surface = VK_NULL_HANDLE;

		if (s_EnabledDebugUtils)
		{
			DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
			m_DebugMessenger = VK_NULL_HANDLE;
		}

		vkDestroyInstance(m_Instance, nullptr);
		m_Instance = VK_NULL_HANDLE;

		AURORA_INFO("Destroyed all vulkan context objects.");
	}


	bool VulkanRenderContext::CreateInstance(
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

		return true;
	}


	bool VulkanRenderContext::CreateSurface(const RenderContextSpecification::SurfaceSpecification& surfaceSpecs)
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
		return true;
	}
	
	bool VulkanRenderContext::PickPhysicalDevice(const DeviceRequirements& deviceRequirements)
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

		return true;
	}

	bool VulkanRenderContext::CreateLogicalDevice(const DeviceRequirements& deviceRequirements)
	{
		QueueFamilyIndices indices = FindQueueFamilies(m_PhysicalDevice);
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

		VkDeviceCreateInfo deviceInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
		deviceInfo.pNext = nullptr;
		deviceInfo.flags = 0;
		deviceInfo.enabledExtensionCount = 0;
		deviceInfo.ppEnabledExtensionNames = nullptr;
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

		return true;
	}


	QueueFamilyIndices VulkanRenderContext::FindQueueFamilies(VkPhysicalDevice phDevice)
	{
		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(phDevice, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilyProps(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(phDevice, &queueFamilyCount, queueFamilyProps.data());

		// We try to find distinct Graphics, Present, Transfer and Compute Queues				
		QueueFamilyIndices indices{};
		//Best case: combined
		bool foundBestPresent = false;
		bool foundUnified = false;
		
		//Best case: dedicated
		bool foundBestTransfer = false;
		bool foundBestCompute = false;	

		int i = 0;
		//Look for UnifiedGraphics queue (with present, graphics and compute support (most integrated GPUs have that))
		for (const auto& family : queueFamilyProps)
		{
			//look until a family was found that supports both present and graphics
			if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT
				&& !foundBestPresent)
			{
				indices.Graphics = i;
			}			
			VkBool32 presentSupport;
			vkGetPhysicalDeviceSurfaceSupportKHR(phDevice, i, m_Surface, &presentSupport);
			if (presentSupport && !foundBestPresent)
			{
				indices.Present = i;
				foundBestPresent = family.queueFlags & VK_QUEUE_GRAPHICS_BIT;
			}
						
			if (!foundBestTransfer && family.queueFlags & VK_QUEUE_TRANSFER_BIT)
			{
				indices.Transfer = i;				
				foundBestTransfer = !(family.queueFlags & VK_QUEUE_GRAPHICS_BIT) && !(family.queueFlags & VK_QUEUE_COMPUTE_BIT);				
			}

			if (!foundBestCompute && family.queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				indices.Compute = i;
				foundBestCompute = !(family.queueFlags & VK_QUEUE_GRAPHICS_BIT);
			}

			//mainly for integrated chips
			if (!foundUnified
				&& family.queueFlags & VK_QUEUE_COMPUTE_BIT 
				&& family.queueFlags & VK_QUEUE_TRANSFER_BIT
				&& foundBestPresent)
			{
				indices.Unified = i;
				indices.UnifiedCount = family.queueCount;
				foundUnified = true;
			}
			i++;
		}
		indices.SamePresentGraphics = foundBestPresent;
		indices.HasDedicatedTransfer = foundBestTransfer;
		indices.HasDedicatedCompute = foundBestCompute;

		return indices;
	}

	// ========== Privat helper ==========

	int VulkanRenderContext::EvaluatePhysicalDevice(VkPhysicalDevice phDevice, const DeviceRequirements& deviceRequirements)
	{
		int score = 0;
		
		QueueFamilyIndices indices = FindQueueFamilies(phDevice);
		if (!indices.IsComplete() || !CheckRequiredDeviceExtensionSupport(phDevice, deviceRequirements))
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

	bool VulkanRenderContext::CheckRequiredDeviceExtensionSupport(VkPhysicalDevice phDevice, const DeviceRequirements& deviceRequirements)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(phDevice, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(phDevice, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		for (const auto& extension : availableExtensions)
		{
			if (requiredExtensions.find(extension.extensionName) != requiredExtensions.end())
			{
				AURORA_INFO("Found required device extension {}", extension.extensionName);
				requiredExtensions.erase(extension.extensionName);
				
				if (requiredExtensions.empty())
					return true;
			}
		}
		for (const auto& extension : requiredExtensions)
		{
			AURORA_ERROR("Required extension {} not supported by device.", extension);
		}

		return false;
	}

	bool VulkanRenderContext::CheckRequiredLayerSupport(const std::vector<const char*>& requiredLayers)
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

	std::vector<const char*> VulkanRenderContext::GetRequiredInstanceExtensions(WSIPlatformType wsi)
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

	bool VulkanRenderContext::CheckRequiredInstanceExtensionsSupport(const std::vector<const char*>& requiredExtensions)
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
	void VulkanRenderContext::SetupDebugMessenger(VkInstance instance, bool enableInfoDebugLevel /*= false*/)
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		PopulateDebugMessengerCreateInfo(createInfo, enableInfoDebugLevel);

		AURORA_VK_CHECK(CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &m_DebugMessenger), VK_SUCCESS, "Failed to create debug messenger.");
	}

	void VulkanRenderContext::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo, bool allowInfoLevel /*= false*/)
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
		createInfo.pfnUserCallback = VulkanDebugCallback;
	}
}
}

