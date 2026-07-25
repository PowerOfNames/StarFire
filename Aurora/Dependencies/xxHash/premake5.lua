project "xxHash"
	kind "StaticLib"
	language "C"
	
	targetdir("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
	
	files
	{
		"xxHash.c"
	}
	
	filter "system:windows"
		systemversion "latest"
		staticruntime "On"
	
	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
	
	