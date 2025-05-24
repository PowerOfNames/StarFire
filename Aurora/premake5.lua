project "Aurora"
	kind "StaticLib"
	language "C++"
	cppdialect "C++23"
	staticruntime "off"

	targetdir("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	
	files
	{
		"include/**.h",
		"src/**.h",
		"src/**.cpp",
		
		"vendor/glm/glm/**.hpp",
		"vendor/glm/glm/**.inl"
	}
	
	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE"
	}
	
	includedirs
	{
		"%{wks.location}/Aurora/include",		
		"%{wks.location}/Aurora/src",		
		"%{IncludeDir.glm}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.VulkanSDK}"
	}
	
	links
	{
		"GLFW",
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
		defines "AURORA_DEBUG"
		runtime "Debug"
		symbols "on"
		
		links
		{
		}
		
	filter "configurations:Release"
		defines "AURORA_RELEASE"
		runtime "Release"
		optimize "on"
		
		links
		{
		}
		
		