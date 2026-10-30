@echo off
pushd %~dp0\..\
call Premake\premake5.exe vs2022 --with-tests
popd
PAUSE
