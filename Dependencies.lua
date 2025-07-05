
Vulkan_SDK = os.getenv("Vulkan_SDK")

IncludeDir = {}
IncludeDir["glm"]					= "%{wks.location}/StarFire/vendor/glm"
IncludeDir["GLFW"]					= "%{wks.location}/StarFire/vendor/GLFW/include"
IncludeDir["Aurora"]				= "%{wks.location}/Aurora/Include"
IncludeDir["spdlog"]				= "%{wks.location}/StarFire/vendor/spdlog/include"
IncludeDir["concurrentqueue"]		= "%{wks.location}/StarFire/vendor/moodycamel"

IncludeDir["VulkanSDK"]				= "%{Vulkan_SDK}/include"
IncludeDir["VMA"]					= "%{wks.location}/Aurora/Vendor/VulkanMemoryAllocator/include"

LibraryDir = {}
LibraryDir["VulkanSDK"]				= "%{Vulkan_SDK}/Lib"
LibraryDir["VulkanSDK_Debug"]		= "%{Vulkan_SDK}/Lib"
LibraryDir["VulkanSDK_DebugDLL"]	= "%{Vulkan_SDK}/Bin"

Library = {}
Library["Vulkan"]					= "%{LibraryDir.VulkanSDK}/vulkan-1.lib"



