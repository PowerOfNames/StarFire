@echo off
REM Compiles every GLSL shader in Aurora\Resources\Shaders to SPIR-V, then regenerates the
REM embedded ShaderByteCodes.h via ShaderByteCodeConverter.exe.
REM To add a shader: drop its .vert/.frag/.comp into Resources\Shaders and run this - no edits needed.

SET "glslcExe=%Vulkan_SDK%\Bin\glslc.exe"

REM This script lives in Aurora\Scripts; resolve the sibling Resources\Shaders folder.
pushd "%~dp0.."
SET "shaderPath=%CD%\Resources\Shaders"
popd

echo Compiling shaders in "%shaderPath%" ...
for %%f in ("%shaderPath%\*.vert" "%shaderPath%\*.frag" "%shaderPath%\*.comp") do (
    echo   %%~nxf
    "%glslcExe%" "%%f" -o "%%f.spv" || goto :error
)

echo.
echo Embedding bytecode into ShaderByteCodes.h ...
REM The converter resolves ..\Resources\Shaders relative to CWD, so run it from this script's dir.
pushd "%~dp0"
"%~dp0ShaderByteCodeConverter.exe" || (popd & goto :error)
popd

echo.
echo Shaders compiled and embedded successfully.
pause
exit /b 0

:error
echo.
echo ERROR: shader compilation/embedding failed.
pause
exit /b 1
