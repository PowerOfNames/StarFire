@echo off
pushd %~dp0\..\
call premake\premake5.exe vs2026 --with-tests
popd
PAUSE
