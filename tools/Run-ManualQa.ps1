param(
    [string]$BuildDir = "build_finish_msvc",
    [string]$Configuration = "Debug",
    [string]$ReportDir = "manual_check_reports",
    [string]$ScreenshotDir = "manual_check_screenshots",
    [switch]$SkipBuild,
    [switch]$NoLaunch
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function New-DirectoryIfMissing {
    param([string]$Path)
    if (-not (Test-Path -LiteralPath $Path)) {
        New-Item -ItemType Directory -Path $Path | Out-Null
    }
}

function Add-ReportLine {
    param(
        [System.Collections.Generic.List[string]]$Lines,
        [string]$Text = ""
    )
    $Lines.Add($Text) | Out-Null
}

function Invoke-NativeStep {
    param(
        [string]$Name,
        [string]$Exe,
        [string[]]$StepArgs,
        [System.Collections.Generic.List[string]]$Lines,
        [bool]$Required = $true
    )

    Add-ReportLine $Lines "## $Name"
    Add-ReportLine $Lines ""
    Add-ReportLine $Lines '```text'
    Add-ReportLine $Lines ($Exe + " " + ($StepArgs -join " "))

    $startInfo = [System.Diagnostics.ProcessStartInfo]::new()
    $startInfo.FileName = $Exe
    $escapedArgs = foreach ($arg in $StepArgs) {
        if ($arg -match '[\s"]') {
            '"' + ($arg -replace '"', '\"') + '"'
        } else {
            $arg
        }
    }
    $startInfo.Arguments = ($escapedArgs -join " ")
    $startInfo.RedirectStandardOutput = $true
    $startInfo.RedirectStandardError = $true
    $startInfo.UseShellExecute = $false
    $startInfo.CreateNoWindow = $true

    $process = [System.Diagnostics.Process]::Start($startInfo)
    $stdout = $process.StandardOutput.ReadToEnd()
    $stderr = $process.StandardError.ReadToEnd()
    $process.WaitForExit()
    $exitCode = $process.ExitCode

    foreach ($line in ($stdout -split "`r?`n")) {
        if ($line.Length -gt 0) {
            Add-ReportLine $Lines $line
        }
    }
    foreach ($line in ($stderr -split "`r?`n")) {
        if ($line.Length -gt 0) {
            Add-ReportLine $Lines $line
        }
    }
    Add-ReportLine $Lines "exit_code=$exitCode"
    Add-ReportLine $Lines '```'
    Add-ReportLine $Lines ""

    if ($Required -and $exitCode -ne 0) {
        throw "$Name failed with exit code $exitCode"
    }

    return $exitCode
}

function Stop-EditorProcesses {
    param([System.Collections.Generic.List[string]]$Lines)

    Add-ReportLine $Lines "## Close Running Editor Processes"
    Add-ReportLine $Lines ""
    foreach ($name in @("BunkerEditor", "BunkerGame", "BunkerLauncher")) {
        $processes = Get-Process $name -ErrorAction SilentlyContinue
        if ($processes) {
            $processes | Stop-Process -Force
            Add-ReportLine $Lines "Stopped $name."
        } else {
            Add-ReportLine $Lines "No running $name process."
        }
    }
    Add-ReportLine $Lines ""
}

function Capture-Screenshot {
    param(
        [string]$Path,
        [System.Collections.Generic.List[string]]$Lines
    )

    Add-Type -AssemblyName System.Windows.Forms
    Add-Type -AssemblyName System.Drawing

    $bounds = [System.Windows.Forms.Screen]::PrimaryScreen.Bounds
    $bitmap = New-Object System.Drawing.Bitmap $bounds.Width, $bounds.Height
    $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
    try {
        $graphics.CopyFromScreen($bounds.Location, [System.Drawing.Point]::Empty, $bounds.Size)
        $bitmap.Save($Path, [System.Drawing.Imaging.ImageFormat]::Png)
        Add-ReportLine $Lines "Screenshot: $Path"
    } finally {
        $graphics.Dispose()
        $bitmap.Dispose()
    }
}

function Launch-Editor {
    param(
        [string]$EditorPath,
        [System.Collections.Generic.List[string]]$Lines
    )

    Add-ReportLine $Lines "## Launch Editor"
    Add-ReportLine $Lines ""
    if (-not (Test-Path -LiteralPath $EditorPath)) {
        throw "Editor executable not found: $EditorPath"
    }

    $process = Start-Process -FilePath $EditorPath -PassThru
    Start-Sleep -Seconds 3

    $activated = $false
    try {
        $shell = New-Object -ComObject WScript.Shell
        $activated = [bool]$shell.AppActivate($process.Id)
    } catch {
        $activated = $false
    }

    Add-ReportLine $Lines "Launched: $EditorPath"
    Add-ReportLine $Lines "pid=$($process.Id)"
    Add-ReportLine $Lines "activated=$activated"
    Add-ReportLine $Lines ""
}

function Add-ManualChecklist {
    param([System.Collections.Generic.List[string]]$Lines)

    Add-ReportLine $Lines "## Manual Checklist"
    Add-ReportLine $Lines ""
    foreach ($item in @(
        "Object Window opens.",
        "No duplicate Object Browser appears.",
        "Object Window is floating/detachable.",
        "Search/filter is visible.",
        "Clear button clears search.",
        "Buttons are aligned and do not cascade chaotically.",
        "Left panel shows category tree.",
        "Right panel shows item list.",
        "Category selection updates item list.",
        "Search filters item list.",
        "Empty categories show a clean empty-state.",
        "Selecting item shows selected item info.",
        "Selecting item does not create/place object.",
        "Render Window opens.",
        "Footer does not show X/Y/Z axis.",
        "LMB selects object.",
        "NumPad8/2/4/6 move selected object camera-relative.",
        "PgUp/PgDn move selected object depth-relative.",
        "- / KeypadSubtract raises selected object on world Z.",
        "+ / KeypadAdd lowers selected object on world Z.",
        "O+NumPad rotation still works.",
        "Shift+P places selected object on support/floor.",
        "NumPad0 and Shift+F focus selected object.",
        "MMB orbit works.",
        "Wheel zoom works.",
        "Drag inside viewport does not drag the whole window.",
        "Cell View / Layers / Warnings open if present."
    )) {
        Add-ReportLine $Lines "- [ ] $item"
    }
    Add-ReportLine $Lines ""
}

$root = (Get-Location).Path
New-DirectoryIfMissing $ReportDir
New-DirectoryIfMissing $ScreenshotDir

$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$reportPath = Join-Path $ReportDir "manual_qa_$timestamp.md"
$screenshotPath = Join-Path $ScreenshotDir "manual_qa_$timestamp.png"
$editorPath = Join-Path $BuildDir (Join-Path $Configuration "BunkerEditor.exe")

$lines = [System.Collections.Generic.List[string]]::new()
Add-ReportLine $lines "# Manual QA Run $timestamp"
Add-ReportLine $lines ""
Add-ReportLine $lines "Project: $root"
Add-ReportLine $lines ""

try {
    Invoke-NativeStep -Name "Git Status" -Exe "git" -StepArgs @("status", "-sb") -Lines $lines | Out-Null
    Invoke-NativeStep -Name "Git Branch" -Exe "git" -StepArgs @("branch", "--show-current") -Lines $lines | Out-Null
    Invoke-NativeStep -Name "Git Latest Commit" -Exe "git" -StepArgs @("log", "--oneline", "--decorate", "-1") -Lines $lines | Out-Null
    Invoke-NativeStep -Name "Diff Stat" -Exe "git" -StepArgs @("--no-pager", "diff", "--stat") -Lines $lines -Required $false | Out-Null
    Invoke-NativeStep -Name "Object Window / Footer String Check" -Exe "rg" -StepArgs @("-n", "ObjectWindow_|###ObjectWindow|Object Browser|X/Y/Z axis", "Editor/src/Editor_Main.cpp") -Lines $lines -Required $false | Out-Null
    Invoke-NativeStep -Name "Diff Check" -Exe "git" -StepArgs @("diff", "--check") -Lines $lines | Out-Null

    Stop-EditorProcesses $lines

    if (-not $SkipBuild) {
        Invoke-NativeStep -Name "Build" -Exe "cmake" -StepArgs @("--build", $BuildDir, "--config", $Configuration) -Lines $lines | Out-Null
        Invoke-NativeStep -Name "CTest" -Exe "ctest" -StepArgs @("--test-dir", $BuildDir, "-C", $Configuration, "--output-on-failure") -Lines $lines | Out-Null
    } else {
        Add-ReportLine $lines "## Build / Test"
        Add-ReportLine $lines ""
        Add-ReportLine $lines "Skipped by -SkipBuild."
        Add-ReportLine $lines ""
    }

    if (-not $NoLaunch) {
        Launch-Editor $editorPath $lines
        Capture-Screenshot $screenshotPath $lines
    } else {
        Add-ReportLine $lines "## Launch Editor"
        Add-ReportLine $lines ""
        Add-ReportLine $lines "Skipped by -NoLaunch."
        Add-ReportLine $lines ""
    }

    Add-ManualChecklist $lines
    Add-ReportLine $lines "Result: automated checks completed. Manual GUI assertions remain pending until verified visually."
} catch {
    Add-ReportLine $lines "## Failure"
    Add-ReportLine $lines ""
    Add-ReportLine $lines "Failure: $($_.Exception.Message)"
    Add-ManualChecklist $lines
    $lines | Set-Content -Path $reportPath -Encoding UTF8
    Write-Host "Manual QA report written: $reportPath"
    throw
}

$lines | Set-Content -Path $reportPath -Encoding UTF8
Write-Host "Manual QA report written: $reportPath"
if (Test-Path -LiteralPath $screenshotPath) {
    Write-Host "Screenshot written: $screenshotPath"
}
