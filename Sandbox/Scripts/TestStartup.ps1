<#
.SYNOPSIS
    Acceptance test for fallible startup (#117). One Sandbox.exe process per case, each checked by its
    exit code and by the CRITICAL log line that names the step that failed.

.DESCRIPTION
    Release and Profiling only. In Debug, STARFIRE_REQUIRE calls __debugbreak() whether or not a debugger
    is attached, so a failure case crashes there instead of exiting with 1. Debug stays a manual pass
    under Visual Studio.

    The working directory is Sandbox/, the folder Visual Studio uses on F5, because the application root
    is still the working directory until the folder-layout milestone.

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

# A folder the current user can read but cannot add subfolders to. SetRootPath's create_directories
# fails there with "access denied", the same situation as an app installed under Program Files.
function New-NoSubfolderDirectory {
    $dir = Join-Path $env:TEMP ('StarFireNoSubfolder_' + [guid]::NewGuid().ToString('N'))
    New-Item -ItemType Directory $dir | Out-Null
    & icacls $dir /deny "*${userSid}:(AD)" | Out-Null
    if ($LASTEXITCODE -ne 0) { throw "icacls could not deny subfolder creation on $dir" }
    return $dir
}

function Remove-NoSubfolderDirectory([string] $Dir) {
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
        [hashtable] $Environment = @{}
    )

    $outLog = Join-Path $logDir "$Config`_$Name.out.log"
    $errLog = Join-Path $logDir "$Config`_$Name.err.log"

    # Environment changes are inherited by the child process and restored afterwards.
    $saved = @{}
    foreach ($key in $Environment.Keys) {
        $saved[$key] = [Environment]::GetEnvironmentVariable($key, 'Process')
        [Environment]::SetEnvironmentVariable($key, $Environment[$key], 'Process')
    }

    try {
        $process = Start-Process -FilePath $Exe -ArgumentList ($Arguments + @('--frames', "$Frames")) `
            -WorkingDirectory $WorkingDirectory -NoNewWindow -PassThru `
            -RedirectStandardOutput $outLog -RedirectStandardError $errLog
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

    $passed = (-not $timedOut) -and ($exitCode -eq $ExpectedExitCode) -and $logFound

    [pscustomobject]@{
        Config   = $Config
        Case     = $Name
        Expected = $ExpectedExitCode
        Actual   = if ($timedOut) { 'timeout' } else { Format-ExitCode $exitCode }
        Log      = if (-not $ExpectedLog) { '-' } elseif ($logFound) { 'found' } else { 'MISSING' }
        Result   = if ($passed) { 'PASS' } else { 'FAIL' }
        Output   = $outLog
    }
}

$missingDriver = Join-Path $env:TEMP 'StarFireNoSuchDriver.json'
$results = @()

foreach ($config in $Configuration) {
    $exe = Join-Path $repoRoot "bin\$config-windows-x86_64\Sandbox\Sandbox.exe"
    if (-not (Test-Path $exe)) {
        Write-Warning "Skipping ${config}: $exe not found. Build Sandbox in $config first."
        continue
    }

    $results += Invoke-Case -Exe $exe -Config $config -Name 'normal-run' -Arguments @() `
        -WorkingDirectory $sandboxDir -ExpectedExitCode 0

    $results += Invoke-Case -Exe $exe -Config $config -Name 'width-zero' -Arguments @('--width', '0') `
        -WorkingDirectory $sandboxDir -ExpectedExitCode 1 -ExpectedLog 'Failed to initialize window!'

    $results += Invoke-Case -Exe $exe -Config $config -Name 'no-vulkan-driver' -Arguments @() `
        -WorkingDirectory $sandboxDir -ExpectedExitCode 1 -ExpectedLog 'Failed to initialize Aurora.' `
        -Environment @{ VK_DRIVER_FILES = $missingDriver; VK_ICD_FILENAMES = $missingDriver }

    # The loader hides every GPU, so the instance, debug messenger and surface exist when device selection
    # fails: a partial Vulkan init that teardown has to clean up.
    $results += Invoke-Case -Exe $exe -Config $config -Name 'no-physical-device' -Arguments @() `
        -WorkingDirectory $sandboxDir -ExpectedExitCode 1 -ExpectedLog 'Failed to pick a physical device' `
        -Environment @{ VK_LOADER_DEVICE_ID_FILTER = '0xFFFE' }

    $results += Invoke-Case -Exe $exe -Config $config -Name 'throw-in-oninit' -Arguments @('--throw-in-oninit') `
        -WorkingDirectory $sandboxDir -ExpectedExitCode 1 -ExpectedLog 'Unhandled exception: Intentional test throw'

    $noSubfolderDir = New-NoSubfolderDirectory
    try {
        $results += Invoke-Case -Exe $exe -Config $config -Name 'root-not-writable' -Arguments @() `
            -WorkingDirectory $noSubfolderDir -ExpectedExitCode 1 -ExpectedLog 'Failed to set root path.'
    }
    finally {
        Remove-NoSubfolderDirectory $noSubfolderDir
    }
}

$results | Format-Table Config, Case, Expected, Actual, Log, Result -AutoSize

$failed = @($results | Where-Object { $_.Result -eq 'FAIL' })
if ($failed.Count -gt 0) {
    Write-Host "Output of failed cases:"
    $failed | ForEach-Object { Write-Host "  $($_.Output)" }
    exit 1
}
exit 0
