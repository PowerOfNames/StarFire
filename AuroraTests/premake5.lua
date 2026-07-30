project "AuroraTests"

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

		-- Aurora's pure Utility headers are header-only, so nothing needs to be
		-- linked except the one out-of-line symbol they reach: Log::Message,
		-- behind the AURORA_WARN macro. Compiling that single self-contained
		-- file avoids depending on Aurora.lib (and therefore on GLFW, ImGui,
		-- shaderc and a Vulkan device) to test pure functions.
		"%{wks.location}/Aurora/Source/Aurora/Core/Logging.cpp",
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		-- Required for the AURORA_* logging macros to expand at all; the
		-- Tests configuration defines none of the AURORA_*_MODE flags itself.
		"AURORA_DEBUG_MODE",
	}

	includedirs
	{
		"Tests/",
		"%{wks.location}/Aurora/Source",
		"%{IncludeDir.Aurora}",
		"%{IncludeDir.Substrate}",
		"%{IncludeDir.Catch2}",
		"%{IncludeDir.VulkanSDK}",
		"%{wks.location}/Aurora/Dependencies/VulkanMemoryAllocator/include",
	}

	links
	{
		-- The Catch2 project itself is included by SubstrateTests, which must
		-- therefore be included before this project in the root premake5.lua.
		"Catch2",
		"%{Library.Vulkan}",
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

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
