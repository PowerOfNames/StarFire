@echo off
pushd %~dp0\..\
call Premake\premake5.exe vs2026
popd
PAUSE
