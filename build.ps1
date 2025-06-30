# MyNotePad 本地构建脚本
# 使用方法: PowerShell -ExecutionPolicy Bypass -File build.ps1

param(
    [string]$Configuration = "Release",
    [string]$Platform = "Win32",
    [switch]$Clean,
    [switch]$Test
)

$ErrorActionPreference = "Stop"

Write-Host "=== MyNotePad 构建脚本 ===" -ForegroundColor Green
Write-Host "配置: $Configuration" -ForegroundColor Yellow
Write-Host "平台: $Platform" -ForegroundColor Yellow

# Check NuGet
Write-Host "Checking NuGet..." -ForegroundColor Yellow
$nuget = $null
$nugetPaths = @(
    "nuget.exe",
    "${env:ProgramFiles(x86)}\NuGet\nuget.exe",
    "${env:ProgramFiles}\NuGet\nuget.exe"
)

foreach ($path in $nugetPaths) {
    if (Get-Command $path -ErrorAction SilentlyContinue) {
        $nuget = $path
        Write-Host "Found NuGet: $nuget" -ForegroundColor Green
        break
    }
}

if (-not $nuget) {
    Write-Host "NuGet not found. Downloading..." -ForegroundColor Yellow
    $nugetUrl = "https://dist.nuget.org/win-x86-commandline/latest/nuget.exe"
    $nugetPath = "$env:TEMP\nuget.exe"
    try {
        Invoke-WebRequest -Uri $nugetUrl -OutFile $nugetPath
        $nuget = $nugetPath
        Write-Host "Downloaded NuGet to: $nuget" -ForegroundColor Green
    }
    catch {
        Write-Warning "Failed to download NuGet. Trying to continue without package restore..."
        $nuget = $null
    }
}

# Check MSBuild
$msbuildPaths = @(
    "${env:ProgramFiles}\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe",
    "${env:ProgramFiles}\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe",
    "${env:ProgramFiles}\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe",
    "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe",
    "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe",
    "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe",
    "${env:ProgramFiles}\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe",
    "${env:ProgramFiles}\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\MSBuild.exe",
    "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe",
    "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\MSBuild.exe"
)

$msbuild = $null
foreach ($path in $msbuildPaths) {
    if (Test-Path $path) {
        $msbuild = $path
        Write-Host "Found MSBuild: $msbuild" -ForegroundColor Green
        break
    }
}

if (-not $msbuild) {
    Write-Error "MSBuild not found. Please install Visual Studio 2019/2022."
    exit 1
}

# Clean build output
if ($Clean) {
    Write-Host "Cleaning build output..." -ForegroundColor Yellow
    $cleanDirs = @("Debug", "Release", "x86", "x64", "Build", "dist", "packages")
    foreach ($dir in $cleanDirs) {
        if (Test-Path $dir) {
            Remove-Item $dir -Recurse -Force
            Write-Host "  Removed: $dir" -ForegroundColor Gray
        }
    }
    
    # Clean project-specific output directories
    $projectDirs = @("MyNotePad", "NotePadWTL")
    foreach ($projectDir in $projectDirs) {
        if (Test-Path $projectDir) {
            $subDirs = @("Debug", "Release", "x86", "x64", "bin", "obj")
            foreach ($subDir in $subDirs) {
                $fullPath = Join-Path $projectDir $subDir
                if (Test-Path $fullPath) {
                    Remove-Item $fullPath -Recurse -Force
                    Write-Host "  Removed: $fullPath" -ForegroundColor Gray
                }
            }
        }
    }
}

# Restore NuGet packages
if ($nuget) {
    Write-Host "Restoring NuGet packages..." -ForegroundColor Yellow
    
    # Check if there are any packages.config files
    $packagesConfigs = Get-ChildItem -Recurse -Name "packages.config" -ErrorAction SilentlyContinue
    if ($packagesConfigs) {
        Write-Host "Found packages.config files, restoring packages..." -ForegroundColor Yellow
        & $nuget restore "MyNotePad.sln"
        if ($LASTEXITCODE -ne 0) {
            Write-Warning "NuGet restore failed, but continuing build..."
        }
        else {
            Write-Host "NuGet packages restored successfully" -ForegroundColor Green
        }
    }
    else {
        Write-Host "No packages.config found, skipping NuGet restore" -ForegroundColor Gray
    }
}
else {
    Write-Warning "NuGet not available, skipping package restore"
}

# Build project
Write-Host "Starting build..." -ForegroundColor Yellow
$buildArgs = @(
    "MyNotePad.sln",
    "/p:Configuration=$Configuration",
    "/p:Platform=$Platform",
    "/m",
    "/verbosity:minimal"
)

& $msbuild @buildArgs

if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed! Exit code: $LASTEXITCODE"
    exit $LASTEXITCODE
}

# Find output files
Write-Host "Looking for build output..." -ForegroundColor Yellow
$possiblePaths = @(
    "$Configuration\MyNotePad.exe",
    "MyNotePad\$Configuration\MyNotePad.exe",
    "$Platform\$Configuration\MyNotePad.exe",
    "Build\MyNotePad.exe",
    "Release\MyNotePad.exe",
    "Debug\MyNotePad.exe"
)

$exePath = $null
foreach ($path in $possiblePaths) {
    if (Test-Path $path) {
        $exePath = $path
        break
    }
}

# Also look for NotePadWTL output
$wtlPossiblePaths = @(
    "$Configuration\NotePadWTL.exe",
    "NotePadWTL\$Configuration\NotePadWTL.exe",
    "$Platform\$Configuration\NotePadWTL.exe",
    "Build\NotePadWTL.exe",
    "Release\NotePadWTL.exe",
    "Debug\NotePadWTL.exe"
)

$wtlExePath = $null
foreach ($path in $wtlPossiblePaths) {
    if (Test-Path $path) {
        $wtlExePath = $path
        break
    }
}

# If we can't find MyNotePad.exe, check if we have NotePadWTL.exe
if (-not $exePath -and $wtlExePath) {
    $exePath = $wtlExePath
    Write-Host "Using NotePadWTL.exe as primary output" -ForegroundColor Yellow
}

if (-not $exePath) {
    Write-Error "Build output not found"
    Write-Host "Searching for all .exe files:" -ForegroundColor Yellow
    Get-ChildItem -Recurse -Include "*.exe" | ForEach-Object { 
        Write-Host "  $($_.FullName)" -ForegroundColor Gray 
    }
    exit 1
}

# Show build results
$file = Get-Item $exePath
Write-Host "Build successful!" -ForegroundColor Green
Write-Host "Output file: $($file.FullName)" -ForegroundColor Green
Write-Host "File size: $($file.Length) bytes" -ForegroundColor Green
Write-Host "Modified: $($file.LastWriteTime)" -ForegroundColor Green

# Create distribution directory
$distDir = "dist"
New-Item -ItemType Directory -Force -Path $distDir | Out-Null

# Copy primary executable
$outputName = if ($exePath -like "*NotePadWTL*") { "NotePadWTL.exe" } else { "MyNotePad.exe" }
Copy-Item $exePath "$distDir\$outputName" -Force

# Copy PDB file if exists
$pdbPath = $exePath -replace "\.exe$", ".pdb"
if (Test-Path $pdbPath) {
    $pdbOutputName = $outputName -replace "\.exe$", ".pdb"
    Copy-Item $pdbPath "$distDir\$pdbOutputName" -Force
    Write-Host "PDB file copied" -ForegroundColor Green
}

# Copy both executables if both exist
if ($exePath -and $wtlExePath -and $exePath -ne $wtlExePath) {
    Write-Host "Found both executables, copying both..." -ForegroundColor Yellow
    Copy-Item $exePath "$distDir\MyNotePad.exe" -Force
    Copy-Item $wtlExePath "$distDir\NotePadWTL.exe" -Force
    
    # Copy both PDB files
    $mynotePdbPath = $exePath -replace "\.exe$", ".pdb"
    $wtlPdbPath = $wtlExePath -replace "\.exe$", ".pdb"
    
    if (Test-Path $mynotePdbPath) {
        Copy-Item $mynotePdbPath "$distDir\MyNotePad.pdb" -Force
    }
    if (Test-Path $wtlPdbPath) {
        Copy-Item $wtlPdbPath "$distDir\NotePadWTL.pdb" -Force
    }
}

Write-Host "Distribution files copied to: $distDir\" -ForegroundColor Green
Get-ChildItem $distDir | ForEach-Object {
    Write-Host "  $($_.Name)" -ForegroundColor Gray
}

# Optional: Run tests
if ($Test) {
    Write-Host "Starting application for testing..." -ForegroundColor Yellow
    $testExe = if (Test-Path "$distDir\MyNotePad.exe") { "$distDir\MyNotePad.exe" } else { "$distDir\NotePadWTL.exe" }
    Start-Process $testExe
}

Write-Host "=== Build Complete ===" -ForegroundColor Green