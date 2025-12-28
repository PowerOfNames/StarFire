@echo off
echo Running all SubstrateTests tests...
pushd %~dp0\..\

if not exist bin\Tests-windows-x86_64\SubstrateTests\SubstrateTests.exe (
	echo Please build and compile SubstarteTests first.
	PAUSE	
) else (
	call "bin\Tests-windows-x86_64\SubstrateTests\SubstrateTests.exe" -b --success --wait-for-keypress "exit"
)
popd

