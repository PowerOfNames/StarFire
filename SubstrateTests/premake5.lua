project "SubstrateTests"

filter {"configurations:Debug or Release or Profiling"}
	kind "None"
filter {"configurations:Tests"}
	kind "ConsoleApp"

	language "C++"
	cppdialect "C++23"
	staticruntime "off"
	rtti "off"

	targetdir("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
	
	files
	{
		"main.cpp",
		"Tests/**.h",
		"Tests/**.cpp",
	}
	
	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
	}
	
	includedirs
	{
		"Tests/",
		"%{IncludeDir.Substrate}",
		"%{SourceDir.Substrate}",
		"%{IncludeDir.Catch2}"
	}
	
	links
	{	
		"Substrate",
		"Catch2",
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
		
		links
		{			
		}
		
	filter "configurations:Release"
		runtime "Release"
		optimize "on"
		
		links
		{
		}
		
				
group "Dependencies"
	include "Dependencies/Catch2"	
group ""