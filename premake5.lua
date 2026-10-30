include "./Premake/customization/solution_items.lua"
include "./Premake/customization/analyze_external.lua"
include "Dependencies.lua"

newoption {
    trigger = "with-tests",
    description = "Include SubstrateTests and AuroraTests projects"
}

workspace "StarFire"
	architecture "x86_64"
	startproject "Sandbox"
	multiprocessorcompile "On"
	externalwarnings "Off"
	analyzeexternal "Off"

	defines
	{
		"GLM_FORCE_DEPTH_ZERO_TO_ONE",
		"GLM_FORCE_INTRINSICS"
	}
	
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

	-- Third-party sources compiled inside a project (e.g. Aurora's SPIRV-Reflect and xxHash).
	-- Must stay in workspace scope: after the first include, premake is inside that project.
	filter "files:**/Dependencies/**"
		warnings "Off"
		enablepch "Off"
	filter {}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
	include "Dependencies/GLFW"
	include "Dependencies/ImGui"
group ""

-- The vendored projects are third-party in full. Re-opening them by name keeps their own scripts untouched.
project "GLFW"
	warnings "Off"
project "ImGui"
	warnings "Off"

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







