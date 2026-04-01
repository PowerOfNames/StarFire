project "Substrate"
	kind "StaticLib"
	language "C++"
	cppdialect "C++23"
	staticruntime "off"
	rtti "off"

	targetdir("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
	
	files
	{
		"Source/**.h",
		"Source/**.cpp",
		"Include/**.h",
	}
	
	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}
	
	includedirs
	{
		"Source",
		"Include",
	}
	
	links
	{	
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
		runtime "Debug"
		symbols "on"
		
		defines
		{
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
			"SUBSTRATE_RELEASE_MODE",
		}		

		links
		{
		}
		
	filter "configurations:Profiling"
		runtime "Release"
		optimize "on"
		
		defines
		{
			"SUBSTRATE_PROFILING_MODE",
			"SUBSTRATE_ENABLE_DETAILS",
			
			"TRACY_ENABLE",
		}
		
		includedirs
		{			
			"%{IncludeDir.Tracy}",
		}
		

		