<#
.SYNOPSIS
    Acceptance test for fallible startup (#117) and the project file system (#119). One Sandbox.exe
    process per case, each checked by its exit code and by the log line that names the step that failed.

.DESCRIPTION
    Release and Profiling only. In Debug, STARFIRE_REQUIRE calls __debugbreak() whether or not a debugger
    is attached, so a failure case crashes there instead of exiting with 1. Debug stays a manual pass
    under Visual Studio.

    Since #119 the application root is no longer the working directory: the project folder is passed with
    --project and validated by a .sfproj marker. Most cases run from Sandbox/ (the folder Visual Studio
    uses on F5) purely for consistency; the 'cwd-independent' case deliberately runs from elsewhere to
    prove the working directory no longer participates.

    Every case also gets --frames, so a case that unexpectedly starts up closes itself instead of hanging.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File Sandbox\Scripts\TestStartup.ps1
    powershell -ExecutionPolicy Bypass -File Sandbox\Scripts\TestStartup.ps1 -Configuration Release
#>
param(
    [ValidateSet('Release', 'Profiling')]
    [string[]] $Configuration = @('Release', 'Profiling'),
    [int] $Frames = 120,
    [int] $TimeoutSeconds = 60
)

$ErrorActionPreference = 'Stop'

$repoRoot   = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$sandboxDir = Join-Path $repoRoot 'Sandbox'
$logDir     = Join-Path $env:TEMP 'StarFireStartupTest'
New-Item -ItemType Directory -Force $logDir | Out-Null

$userSid = [System.Security.Principal.WindowsIdentity]::GetCurrent().User.Value

# Throwaway project folder. -WithMarker adds the .sfproj that FileSystem::SetProjectDir requires;
# -WithAssets adds the full Assets tree that FileSystem::CheckupAssetPaths requires; -WithAssetRoot adds
# only Assets/ and deliberately omits Assets/Shaders. Leaving a piece out is how each failure case is
# produced.
function New-TestProject {
    param([switch] $WithMarker, [switch] $WithAssets, [switch] $WithAssetRoot)

    $dir = Join-Path $env:TEMP ('StarFireProject_' + [guid]::NewGuid().ToString('N'))
    New-Item -ItemType Directory $dir | Out-Null

    if ($WithMarker) {
        # Matches Sandbox.sfproj's format. Nothing parses the contents yet - only the extension and the
        # filename stem are used - but the fixture should not invent a second convention.
        $marker = Join-Path $dir 'TestProject.sfproj'
        Set-Content -Path $marker -Value @('ProjectName TestProject', 'Version 0.0.0.1') -Encoding utf8
    }
    if ($WithAssets) {
        New-Item -ItemType Directory (Join-Path $dir 'Assets\Shaders') -Force | Out-Null
    }
    elseif ($WithAssetRoot) {
        New-Item -ItemType Directory (Join-Path $dir 'Assets') -Force | Out-Null
    }
    return $dir
}

# Deny subfolder creation for the current user, so create_directories fails with "access denied" - the
# same situation as a project folder sitting under Program Files.
function Deny-SubfolderCreation([string] $Dir) {
    & icacls $Dir /deny "*${userSid}:(AD)" | Out-Null
    if ($LASTEXITCODE -ne 0) { throw "icacls could not deny subfolder creation on $Dir" }
}

function Remove-TestDirectory([string] $Dir) {
    if (-not (Test-Path $Dir)) { return }
    & icacls $Dir /remove:d "*$userSid" | Out-Null
    Remove-Item -Recurse -Force $Dir
}

function Format-ExitCode($Code) {
    if ($null -eq $Code) { return 'none' }
    if ($Code -lt 0 -or $Code -gt 255) { return ('0x{0:X8}' -f $Code) }
    return "$Code"
}

function Invoke-Case {
    param(
        [string]    $Exe,
        [string]    $Config,
        [string]    $Name,
        [string[]]  $Arguments,
        [string]    $WorkingDirectory,
        [int]       $ExpectedExitCode,
        [string]    $ExpectedLog,
        [string]    $AssertMissing,
        [hashtable] $Environment = @{}
    )

    $outLog = Join-Path $logDir ($Config + '_' + $Name + '.out.log')
    $errLog = Join-Path $logDir ($Config + '_' + $Name + '.err.log')

    # Environment changes are inherited by the child process and restored afterwards.
    $saved = @{}
    foreach ($key in $Environment.Keys) {
        $saved[$key] = [Environment]::GetEnvironmentVariable($key, 'Process')
        [Environment]::SetEnvironmentVariable($key, $Environment[$key], 'Process')
    }

    try {
        $allArgs = $Arguments + @('--frames', "$Frames")
        $process = Start-Process -FilePath $Exe -ArgumentList $allArgs -WorkingDirectory $WorkingDirectory -NoNewWindow -PassThru -RedirectStandardOutput $outLog -RedirectStandardError $errLog
        $null = $process.Handle   # without this, ExitCode stays empty for a -PassThru process

        $timedOut = -not $process.WaitForExit($TimeoutSeconds * 1000)
        if ($timedOut) { $process.Kill() }
        $process.WaitForExit()
        $exitCode = if ($timedOut) { $null } else { $process.ExitCode }
    }
    finally {
        foreach ($key in $saved.Keys) {
            [Environment]::SetEnvironmentVariable($key, $saved[$key], 'Process')
        }
    }

    $logFound = $true
    if ($ExpectedLog) {
        $logFound = [bool](Select-String -Path $outLog, $errLog -SimpleMatch $ExpectedLog -Quiet)
    }

    # Nothing may be created inside a folder that failed validation.
    $notCreated = $true
    if ($AssertMissing) { $notCreated = -not (Test-Path $AssertMissing) }

    $passed = (-not $timedOut) -and ($exitCode -eq $ExpectedExitCode) -and $logFound -and $notCreated

    [pscustomobject]@{
        Config   = $Config
        Case     = $Name
        Expected = $ExpectedExitCode
        Actual   = if ($timedOut) { 'timeout' } else { Format-ExitCode $exitCode }
        Log      = if (-not $ExpectedLog) { '-' } elseif ($logFound) { 'found' } else { 'MISSING' }
        NoWrite  = if (-not $AssertMissing) { '-' } elseif ($notCreated) { 'clean' } else { 'CREATED' }
        Result   = if ($passed) { 'PASS' } else { 'FAIL' }
        Output   = $outLog
    }
}

$missingDriver  = Join-Path $env:TEMP 'StarFireNoSuchDriver.json'
$missingProject = Join-Path $env:TEMP 'StarFireNoSuchProject'
$results = @()

foreach ($config in $Configuration) {
    $exe = Join-Path $repoRoot "bin\$config-windows-x86_64\Sandbox\Sandbox.exe"
    if (-not (Test-Path $exe)) {
        Write-Warning "Skipping ${config}: $exe not found. Build Sandbox in $config first."
        continue
    }

    $project = @('--project', $sandboxDir)

    # ---- startup succeeds ----

    $results += Invoke-Case -Exe $exe -Config $config -Name 'normal-run' -Arguments $project -WorkingDirectory $sandboxDir -ExpectedExitCode 0

    # The working directory is deliberately somewhere else: --project alone must determine the root.
    $results += Invoke-Case -Exe $exe -Config $config -Name 'cwd-independent' -Arguments $project -WorkingDirectory $env:TEMP -ExpectedExitCode 0

    # ---- project folder failures (#119) ----

    $results += Invoke-Case -Exe $exe -Config $config -Name 'no-project' -Arguments @() -WorkingDirectory $sandboxDir -ExpectedExitCode 1 -ExpectedLog 'Failed to set project path.'

    $results += Invoke-Case -Exe $exe -Config $config -Name 'project-missing' -Arguments @('--project', $missingProject) -WorkingDirectory $sandboxDir -ExpectedExitCode 1 -ExpectedLog 'Failed to set project path.'

    # No .sfproj: rejected before anything is created, so the folder must stay untouched.
    $noMarkerDir = New-TestProject
    try {
        $results += Invoke-Case -Exe $exe -Config $config -Name 'project-no-marker' -Arguments @('--project', $noMarkerDir) -WorkingDirectory $sandboxDir -ExpectedExitCode 1 -ExpectedLog 'Project file does not contain required .sfproj file!' -AssertMissing (Join-Path $noMarkerDir 'Assets')
    }
    finally { Remove-TestDirectory $noMarkerDir }

    # Marker present but no Assets tree: Aurora rejects it, and must not create Assets itself.
    $noAssetsDir = New-TestProject -WithMarker
    try {
        $results += Invoke-Case -Exe $exe -Config $config -Name 'project-no-assets' -Arguments @('--project', $noAssetsDir) -WorkingDirectory $sandboxDir -ExpectedExitCode 1 -ExpectedLog 'Some asset directories could not be resolved.' -AssertMissing (Join-Path $noAssetsDir 'Assets')
    }
    finally { Remove-TestDirectory $noAssetsDir }

    # Assets/ present but Assets/Shaders missing. Pins the second half of CheckupAssetPaths, which is the
    # half a copy-paste can silently skip.
    $noShaderDir = New-TestProject -WithMarker -WithAssetRoot
    try {
        $results += Invoke-Case -Exe $exe -Config $config -Name 'project-no-shader-dir' -Arguments @('--project', $noShaderDir) -WorkingDirectory $sandboxDir -ExpectedExitCode 1 -ExpectedLog 'Some asset directories could not be resolved.' -AssertMissing (Join-Path $noShaderDir 'Assets\Shaders')
    }
    finally { Remove-TestDirectory $noShaderDir }

    # A valid project the process may read but not write: Cache/ creation fails.
    $readOnlyDir = New-TestProject -WithMarker -WithAssets
    try {
        Deny-SubfolderCreation $readOnlyDir
        $results += Invoke-Case -Exe $exe -Config $config -Name 'cache-not-writable' -Arguments @('--project', $readOnlyDir) -WorkingDirectory $sandboxDir -ExpectedExitCode 1 -ExpectedLog 'Failed to create cache directories.'
    }
    finally { Remove-TestDirectory $readOnlyDir }

    # ---- startup failures inherited from #117 ----

    $results += Invoke-Case -Exe $exe -Config $config -Name 'width-zero' -Arguments ($project + @('--width', '0')) -WorkingDirectory $sandboxDir -ExpectedExitCode 1 -ExpectedLog 'Failed to initialize window!'

    $results += Invoke-Case -Exe $exe -Config $config -Name 'no-vulkan-driver' -Arguments $project -WorkingDirectory $sandboxDir -ExpectedExitCode 1 -ExpectedLog 'Failed to initialize Aurora.' -Environment @{ VK_DRIVER_FILES = $missingDriver; VK_ICD_FILENAMES = $missingDriver }

    # The loader hides every GPU, so the instance, debug messenger and surface exist when device selection
    # fails: a partial Vulkan init that teardown has to clean up.
    $results += Invoke-Case -Exe $exe -Config $config -Name 'no-physical-device' -Arguments $project -WorkingDirectory $sandboxDir -ExpectedExitCode 1 -ExpectedLog 'Failed to pick a physical device' -Environment @{ VK_LOADER_DEVICE_ID_FILTER = '0xFFFE' }

    $results += Invoke-Case -Exe $exe -Config $config -Name 'throw-in-oninit' -Arguments ($project + @('--throw-in-oninit')) -WorkingDirectory $sandboxDir -ExpectedExitCode 1 -ExpectedLog 'Unhandled exception: Intentional test throw'
}

$results | Format-Table Config, Case, Expected, Actual, Log, NoWrite, Result -AutoSize

$failed = @($results | Where-Object { $_.Result -eq 'FAIL' })
if ($failed.Count -gt 0) {
    Write-Host "Output of failed cases:"
    $failed | ForEach-Object { Write-Host "  $($_.Output)" }
    exit 1
}
exit 0
