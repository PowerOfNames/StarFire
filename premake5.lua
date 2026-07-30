include "./Premake/customization/solution_items.lua"
include "Dependencies.lua"

newoption {
    trigger = "with-tests",
    description = "Include SubstrateTests and AuroraTests projects"
}

workspace "StarFire"
	architecture "x86_64"
	startproject "Sandbox"
	multiprocessorcompile "On"
	
	configurations 
	{
		"Debug",
		"Release",
		"Tests",
		"Profiling",
	}

	solution_items
	{
		".editorconfig"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
	include "Dependencies/GLFW"
	include "Dependencies/ImGui"
group ""

filter {"files/Dependencies/**.cpp"}
	warnings "Off"
filter {}

filter {"files/Dependencies/**.cpp"}
	enablepch "Off"
filter{}

if _OPTIONS["with-tests"] then
include "Substrate"
-- SubstrateTests must come before AuroraTests: it is what pulls in the shared
-- Catch2 project, which AuroraTests links against.
include "SubstrateTests"
include "AuroraTests"
else
	include "StarFire"
	include "Aurora"
	include "Sandbox"
	include "Nebula"
	include "Substrate"
end







