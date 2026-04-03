
Vulkan_SDK = os.getenv("Vulkan_SDK")

IncludeDir = {}
IncludeDir["Aurora"]				= "%{wks.location}/Aurora/Include"
IncludeDir["Substrate"]				= "%{wks.location}/Substrate/Include"

--For unit testing only
SourceDir = {}
SourceDir["Substrate"]				= "%{wks.location}/Substrate/Source"

IncludeDir["glm"]					= "%{wks.location}/Dependencies/glm"
IncludeDir["GLFW"]					= "%{wks.location}/Dependencies/GLFW/include"
IncludeDir["spdlog"]				= "%{wks.location}/Dependencies/spdlog/include"

IncludeDir["Catch2"]				= "%{wks.location}/SubstrateTests/Dependencies/Catch2/src"
IncludeDir["Tracy"]					= "%{wks.location}/Dependencies/Tracy/public"

IncludeDir["VulkanSDK"]				= "%{Vulkan_SDK}/include"
LibraryDir = {}
LibraryDir["VulkanSDK"]				= "%{Vulkan_SDK}/Lib"
LibraryDir["VulkanSDK_Debug"]		= "%{Vulkan_SDK}/Lib"
LibraryDir["VulkanSDK_DebugDLL"]	= "%{Vulkan_SDK}/Bin"

Library = {}
Library["Vulkan"]					= "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["shaderc_debug"]			= "%{LibraryDir.VulkanSDK_Debug}/shaderc_sharedd.lib"

Library["shaderc_release"]			= "%{LibraryDir.VulkanSDK}/shaderc_shared.lib"



