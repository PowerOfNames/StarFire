@echo off
pushd %~dp0\..\

if not exist bin\Tests-windows-x86_64\SubstrateTests\SubstrateTests.exe (
	echo Please build and compile SubstarteTests first.
	PAUSE	
) else (
	setlocal enabledelayedexpansion
	set /p tags= Please write the tags of the tests you want to run. Only tests that contain all of these tags are considered:
	set "wrapped_tags="
	for %%w in (!tags!) do (
		set "wrapped_tags=!wrapped_tags! [%%w]"
	)
	echo Tags entered: !tags!
	echo Wrapped tags: !wrapped_tags!

	call "bin\Tests-windows-x86_64\SubstrateTests\SubstrateTests.exe" !wrapped_tags! -b --success --wait-for-keypress "exit"
)

popd

