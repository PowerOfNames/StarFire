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
group ""

include "StarFire"
include "Sandbox"
include "Nebula"







