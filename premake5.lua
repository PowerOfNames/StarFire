include "./premake/customization/solution_items.lua"
include "Dependencies.lua"

newoption {
    trigger = "with-tests",
    description = "Include SubstrateTests project"
}

workspace "StarFire"
	architecture "x86_64"
	startproject "Sandbox"

	configurations 
	{
		"Debug",
		"Release",
		"Tests"
	}

	solution_items
	{
		".editorconfig"
	}

	flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
	include "StarFire/vendor/GLFW"
	include "StarFire/vendor/xxHash"	
group ""

filter {"StarFire/vendor/**.cpp"}
	warnings "Off"
filter {}

filter {"files/vendor/**.cpp"}
	flags {"NoPCH"}
filter{}

if _OPTIONS["with-tests"] then
group "Core"
group ""
group "Shared"
	include "Substrate"
group ""
group "Tests"
    include "SubstrateTests"
group ""
else
group "Core"
	include "StarFire"
	include "Aurora"
	include "Sandbox"
	include "Nebula"
group ""
group "Shared"
	include "Substrate"
group ""
group "Tests"
group ""
end







