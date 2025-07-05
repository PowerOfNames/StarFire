@echo off
SET "vulkanSDK=%Vulkan_SDK%"

SET "glslcExe=%vulkanSDK%\Bin\glslc.exe"

SET "WorkingDir=%~dp0"
CD /D "%WorkingDir%""
CD ..

SET "shaderPath=%CD%\Resources\Shaders"

"%glslcExe%" "%shaderPath%\SwapchainFallback.vert" -o "%shaderPath%\SwapchainFallback.vert.spv"
"%glslcExe%" "%shaderPath%\SwapchainFallback.frag" -o "%shaderPath%\SwapchainFallback.frag.spv"

echo.
echo Shaders compiled successfully
pause