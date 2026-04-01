
Vulkan_SDK = os.getenv("Vulkan_SDK")

IncludeDir = {}
IncludeDir["Aurora"]				= "%{wks.location}/Aurora/Include"
IncludeDir["Substrate"]				= "%{wks.location}/Substrate/Include"

--For unit testing only
SourceDir = {}
SourceDir["Substrate"]				= "%{wks.location}/Substrate/Source"

IncludeDir["glm"]					= "%{wks.location}/StarFire/vendor/glm"
IncludeDir["GLFW"]					= "%{wks.location}/StarFire/vendor/GLFW/include"
IncludeDir["spdlog"]				= "%{wks.location}/StarFire/vendor/spdlog/include"
IncludeDir["concurrentqueue"]		= "%{wks.location}/StarFire/vendor/moodycamel"
IncludeDir["xxHash"]				= "%{wks.location}/StarFire/vendor/xxHash"
IncludeDir["Catch2"]				= "%{wks.location}/SubstrateTests/Dependencies/Catch2/src"
IncludeDir["Tracy"]					= "%{wks.location}/Dependencies/Tracy/public"

IncludeDir["VulkanSDK"]				= "%{Vulkan_SDK}/include"
IncludeDir["VMA"]					= "%{wks.location}/Aurora/Vendor/VulkanMemoryAllocator/include"
IncludeDir["Spirv_Reflect"]			= "%{wks.location}/Aurora/Vendor/SPIRV-Reflect"

LibraryDir = {}
LibraryDir["VulkanSDK"]				= "%{Vulkan_SDK}/Lib"
LibraryDir["VulkanSDK_Debug"]		= "%{Vulkan_SDK}/Lib"
LibraryDir["VulkanSDK_DebugDLL"]	= "%{Vulkan_SDK}/Bin"

Library = {}
Library["Vulkan"]					= "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["shaderc_debug"]			= "%{LibraryDir.VulkanSDK_Debug}/shaderc_sharedd.lib"

Library["shaderc_release"]			= "%{LibraryDir.VulkanSDK}/shaderc_shared.lib"



