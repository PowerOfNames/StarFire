project "Aurora"
	kind "StaticLib"
	language "C++"
	cppdialect "C++23"
	staticruntime "off"

	targetdir("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	
	files
	{
		"Source/**.h",
		"Source/**.cpp",
		"Include/**.h",
		"Resources/**.h",
		
		"vendor/glm/glm/**.hpp",
		"vendor/glm/glm/**.inl",
		
		"vendor/SPIRV-Reflect/spirv_reflect.cpp",
		"vendor/SPIRV-Reflect/spirv_reflect.h"
	}
	
	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE"
	}
	
	includedirs
	{
		"Source",
		"Include",
		"Resources",
		"%{IncludeDir.glm}",
		"%{IncludeDir.Substrate}",
		"%{IncludeDir.xxHash}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.VulkanSDK}",
		"%{IncludeDir.VMA}",
		"%{IncludeDir.Spirv_Reflect}"
	}
	
	links
	{
		"Substrate",
		"GLFW",
		"xxHash",
		"%{Library.Vulkan}"
	}
	
	
	filter "system:windows"
		systemversion "latest"
		
	filter "action:vs*"
		buildoptions
		{
			"/utf-8"
		}
		characterset "Unicode"
	
	filter "configurations:Debug"
		defines
		{
			"AURORA_DEBUG_MODE",
			"SUBSTRATE_DEBUG_MODE",
			"SUBSTRATE_ENABLE_DETAILS",
		}
		runtime "Debug"
		symbols "on"
		
		links
		{
			"%{Library.shaderc_debug}"
		}
		
	filter "configurations:Release"
		defines
		{
			"AURORA_RELEASE_MODE",
			"SUBSTRATE_RELEASE_MODE",
		}
		runtime "Release"
		optimize "on"
		
		links
		{
			"%{Library.shaderc_release}"
		}
		
	filter "configurations:Profiling"
		runtime "Release"
		optimize "on"
		
		defines
		{
			"AURORA_PROFILING_MODE",
			"SUBSTRATE_PROFILING_MODE",
			"SUBSTRATE_ENABLE_DETAILS",
			
			"TRACY_ENABLE",		
		}
		
		includedirs
		{			
			"%{IncludeDir.Tracy}",
		}
		
		