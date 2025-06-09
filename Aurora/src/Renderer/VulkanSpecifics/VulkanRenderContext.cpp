#include "Renderer/VulkanSpecifics/VulkanCore.h"
#include "Renderer/VulkanSpecifics/VulkanRenderContext.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


namespace Aurora { namespace VK {

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
			AURORA_ERROR("Failed to create instance! VulkanContext could not be initialized!");
			return;
		}

		if (!CreateSurface(m_Specification.SurfaceSpecs))
		{
			AURORA_ERROR("Failed to create surface! VulkanContext could not be instanziated!");
			return;
		}



		DeviceRequirements deviceReqs{};
		ChoosePhysicalDevice(deviceReqs);
		CreatePhysicalDevice();
		CreateLogicalDevice();
	}


	void VulkanRenderContext::Shutdown()
	{


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
#ifdef AURORA_VK_VALIDATION
			"VK_LAYER_KHRONOS_validation",
			"VK_LAYER_KHRONOS_synchronization2"
#endif
		};
		if (!CheckRequiredLayerSupport(requiredLayers))
		{
			AURORA_ERROR("Some required layers are not available! Instance will not be created.");
			return false;
		}

		// ===== Extensions =====
		std::vector<const char*> requiredExtensions = GetRequiredExtensions(wsi);	

#ifdef AURORA_VK_VALIDATION
		bool useDebugUtils = instanceSpecs.EnableDebugUtils;
#else
		bool useDebugUtils = false;
#endif 
		if (useDebugUtils)
			requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		if (!CheckRequiredExtensionsSupport(requiredExtensions))
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
			PopulateDebugMessengerCreateInfo(debugUtilsInfo);
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


#ifdef AURORA_VK_VALIDATION
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


	bool VulkanRenderContext::ChoosePhysicalDevice(const DeviceRequirements& deviceRequirements)
	{

		return true;
	}


	bool VulkanRenderContext::CreatePhysicalDevice()
	{

		return true;
	}


	bool VulkanRenderContext::CreateLogicalDevice()
	{

		return true;
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
				AURORA_ERROR("Requested layer {} not available!", layer);
			}
		}
		return allLayerFound;
	}

	std::vector<const char*> VulkanRenderContext::GetRequiredExtensions(WSIPlatformType wsi)
	{
		std::vector<const char*> extensions;
		if (wsi == WSIPlatformType::SURFACE_PLATFORM_GLFW)
		{
			uint32_t glfwExtensionCount;
			const char** glfwExtensions;
			glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
			extensions.insert(extensions.end(), glfwExtensions, (glfwExtensions+glfwExtensionCount));
		}
		return extensions;
	}

	bool VulkanRenderContext::CheckRequiredExtensionsSupport(const std::vector<const char*>& requiredExtensions)
	{


		return true;
	}

	//Global messenger for AFTER instance creation
	void VulkanRenderContext::SetupDebugMessenger(VkInstance instance)
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		PopulateDebugMessengerCreateInfo(createInfo);

		AURORA_VK_CHECK(CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &m_DebugMessenger), VK_SUCCESS, "Failed to create debug messenger.");
	}

	void VulkanRenderContext::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;

		VkDebugUtilsMessageSeverityFlagsEXT severityFlags = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT 
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
//#ifdef AURORA_DEBUG_MODE
//		severityFlags |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
//#endif
		createInfo.messageSeverity = severityFlags;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT 
			| VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT 
			| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = VulkanDebugCallback;		
	}
}
}

