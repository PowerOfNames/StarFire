project "Nebula"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++23"
	staticruntime "off"

	targetdir("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
	
	files
	{
		"src/**.h",
		"src/**.cpp"
	}
	
	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}
	
	includedirs
	{
		"%{wks.location}/StarFire/src",
		"%{IncludeDir.glm}",
		"%{IncludeDir.spdlog}"
	}
	
	links
	{
		"StarFire"
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
		defines "SF_DEBUG"
		runtime "Debug"
		symbols "on"
		
		links
		{			
		}
		
	filter "configurations:Release"
		defines "SF_RELEASE"
		runtime "Release"
		optimize "on"
		
		links
		{
		}
		
		