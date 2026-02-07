project "Nebula"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++23"
	staticruntime "off"

	targetdir("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
	
	files
	{
		"Source/**.h",
		"Source/**.cpp"
	}
	
	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}
	
	includedirs
	{
		"%{wks.location}/StarFire/Source",
		"%{IncludeDir.Aurora}",
		"%{IncludeDir.Substrate}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.spdlog}",
	}
	
	links
	{
		"StarFire",
		"Aurora",
		"Substrate",
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
		defines
		{
			"STARFIRE_DEBUG_MODE",
			"SUBSTRATE_DEBUG_MODE",
			"SUBSTRATE_ENABLE_DETAILS",
		}		
		runtime "Debug"
		symbols "on"
		
		links
		{			
		}
		
	filter "configurations:Release"
		defines
		{
			"STARFIRE_RELEASE_MODE",
			"SUBSTRATE_RELEASE_MODE",
		}
		runtime "Release"
		optimize "on"
		
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
		
		files
		{
			path.join("%{IncludeDir.Tracy}", "TracyClient.cpp"),
		}
		
	filter "files:**/TracyClient.cpp"	
		flags {"NoPCH"}
	filter {}
		
		