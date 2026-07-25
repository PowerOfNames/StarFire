project "StarFire"
	kind "StaticLib"
	language "C++"
	cppdialect "C++23"
	staticruntime "off"

	targetdir("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "sfpch.h"
	pchsource "Source/sfpch.cpp"
		
	files
	{
		"Source/**.h",
		"Source/**.cpp",
	}
	
filter "files:**/ImGuiBuild.cpp"
	enablepch "Off"
filter{}
	
	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE",
		"AURORA_GLFW",
	}
	
	includedirs
	{
		"Source",
		"%{IncludeDir.Aurora}",
		"%{IncludeDir.Substrate}",
		
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.spdlog}",
		
		"Dependencies/moodycamel",
	}
	
	links
	{
		"Aurora",
		"GLFW",
		"ImGui",
	}
	
	
	filter "system:windows"
		systemversion "latest"
		defines
		{
			"STARFIRE_PLATFORM_WINDOWS",
		}
		
	filter "action:vs*"
		buildoptions
		{
			"/utf-8"
		}
		characterset "Unicode"
	
	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"
		editandcontinue "Off"
		
		defines
		{
			"STARFIRE_DEBUG_MODE",
			"SUBSTRATE_DEBUG_MODE",
			"SUBSTRATE_ENABLE_DETAILS",
		}
		
		links
		{			
		}
		
	filter "configurations:Release"
		runtime "Release"
		optimize "on"
		
		defines
		{
			"STARFIRE_RELEASE_MODE",
			"SUBSTRATE_RELEASE_MODE",
		}
		
		links
		{
		}
		
	filter "configurations:Profiling"
		defines
		{
			"STARFIRE_PROFILING_MODE",
			"SUBSTRATE_PROFILING_MODE",
			"SUBSTRATE_ENABLE_DETAILS",
			
			"TRACY_ENABLE",
		}
		
		runtime "Release"
		optimize "on"
		
		includedirs
		{			
			"%{IncludeDir.Tracy}",
		}