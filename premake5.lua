include "./premake/customization/solution_items.lua"
include "Dependencies.lua"

workspace "StarFire"
	architecture "x86_64"
	startproject "Sandbox"

	configurations 
	{
		"Debug",
		"Release"
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

group "Core"
	include "StarFire"
	include "Aurora"
	include "Sandbox"
	include "Nebula"
	include "Substrate"
group ""







