@echo off

SET "WorkingDir=%~dp0"

echo Script dir: "%WorkingDir%"
echo VulkanSDK path: "%Vulkan_SDK%"
echo GLSLC path: "%Vulkan_SDK%\Bin\glslc.exe"

CD /D "%WorkingDir%""
CD ..

echo Shaders path: "%CD%\Resources\Shaders"
pause