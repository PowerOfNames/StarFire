# StarFire
StarFire is a "to-be-in-the-future" 2D | Isometric 3D Visualization Engine, with a custom Vulkan rendering backend, ImGui editor user interface. The plan is to produce small, colorful, simulation heavy games and other fun projects with it.

This will be the second iteration of the engine-design architecture learning process, featuring a full functional, Vulkan rendering backend utilizing compute shaders as backbone for heavy simulations, an optimized pipeline for 2D, and later isometric/three dimensional, applications. It will support a small 3D sound library, custom shaders, ray-traced lighting focused on utilizing new gen hardware. 

StarFire will be a long term project, which will be extended as needed, where the side projects give direction on features.


## Requirements
StarFire is build using [premake5](https://premake.github.io/download).

Visual Studio 2022, C++23, x64

[VulkanSDK](https://vulkan.lunarg.com/sdk/home#windows) Version 1.3

StarFire currently only supports Windows.

## Dependencies
|Name|Version|Repo|Branch|Commit|Last Updated|Used in project|Config|
|---|---|---|---|---|---|---|---|
|[glm](https://github.com/g-truc/glm/tree/2d4c4b4dd31fde06cfffad7915c2b3006402322f)|1.0.1|Original|master|'2d4c4b4'|21.04.2025|StarFire;Aurora;Nebula;Sandbox|all|
|[GLFW](https://github.com/PowerOfNames/glfw/tree/06d9a46f5b0bb4ffed91a87411bfa051f03b95f6)|3.5.0|Forked|StarFire|'06d9a46'|21.04.2025|StarFire;Aurora|all|
|[spdlog](https://github.com/gabime/spdlog/tree/847db3375f35dca23e45f9daf7ad3c7b19027f8b)|1.25.2|Original|v1.x|'847db33'|21.04.2025|StarFire;Nebula;Sandbox|all|
|[concurrentqueue](https://github.com/cameron314/concurrentqueue/tree/24b78782bd6ca5a5853ef46917708806112dc142)|1.0.4|Original|master|'24b7878'|14.05.2025|StarFire|all|
|[VulkanSDK](https://vulkan.lunarg.com/sdk/home#windows)|1.3.296(latest 1.3)|Original|---|---|24.05.2025|Aurora|all|
|[xxHash](https://github.com/Cyan4973/xxHash)|v0.8.3|Original|---|---|17.07.2025|Aurora/StarFire|all|
|[catch2](https://github.com/catchorg/Catch2)|v3.11.0|Forked|[v3.11.0_Forked](https://github.com/PowerOfNames/Catch2/tree/v3.11.0_forked)|'31d46ee'|07.12.2025|SubstrateTests|Tests|
|[Tracy](https://github.com/wolfpld/tracy)|v.0.13.1|Original|---|---|04.02.2026|StarFire;Aurora;Nebula;Sandbox|all(toggleable by #define)|

Disclaimer: The branches in GLFW were a test. There is no difference between the premake5 files in this project and the old engine [Povox](https://github.com/PowerOfNames/Povox).

## How to Build
1. Clone the repository. 
  - git clone --recursive https://github.com/PowerOfNames/StarFire or
  - git clone https://github.com/PowerOfNames/StarFire + git submodule update --init --recursive
2. Install the latest 1.3.xxx VulkanSDK for Windows 64 bit and set the system environment variable (default: Vulkan_SDK)
3. Navigate to /scripts/build and run win-genProjects. You might need to change the respective line in the scripts if premake5.exe was moved.
4. Open the StarFire solution. Sandbox is selected as StatUp project by default.

### Building Tracy
1. Follow their instructions using cmake, in case profiling is needed [Documentation_2.3_Building_the_server](https://github.com/wolfpld/tracy?tab=readme-ov-file)
2. Run tracy-profiler.exe that was generated into profiler/build/Release (or in whichever path you set)
3. Start a StarFire application in (config:Profiling)


