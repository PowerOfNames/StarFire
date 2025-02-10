# StarFire
StarFire is a "to-be-in-the-future" 2D | Isometric 3D Visualization Engine, with a custom Vulkan rendering backend, ImGui editor user interface. The plan is to produce small, colorful, simulation heavy games and other fun projects with it.

This will be the second iteration of the engine-design architecture learning process, featuring a full functional, Vulkan rendering backend utilizing compute shaders as backbone for heavy simulations, an optimized pipeline for 2D, and later isometric/three dimensional, applications. It will support a small 3D sound library, custom shaders, ray-traced lighting focused on utilizing new gen hardware. 

StarFire will be a long term project, which will be extended as needed, where the side projects give direction on features.


## Requirements
StarFire is build using [premake5](https://premake.github.io/download).

Visual Studio 2022, C++20, x64

[VulkanSDK](https://vulkan.lunarg.com/sdk/home#windows) Version 1.3+ (Planning to switch to 1.4.x at some point, when needed)

StarFire currently only supports Windows.

## How to Build
1. Clone the repository. 
  - git clone --recursive https://github.com/PowerOfNames/StarFire or
  - git clone https://github.com/PowerOfNames/StarFire + git submodule update --init
2. Navigate to /scripts/build and run win-genProjects. You might need to change the respective line in the scripts to whereever you put your premake5.exe
3. Open the StarFire solution and run. Make sure that VulkanSDK is set correctly as a system path variable.
