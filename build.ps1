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

# Check MSBuild
$msbuildPaths = @(
    "${env:ProgramFiles}\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe",
    "${env:ProgramFiles}\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe",
    "${env:ProgramFiles}\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe",
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
    $cleanDirs = @("Debug", "Release", "x86", "x64", "Build", "dist")
    foreach ($dir in $cleanDirs) {
        if (Test-Path $dir) {
            Remove-Item $dir -Recurse -Force
            Write-Host "  Removed: $dir" -ForegroundColor Gray
        }
    }
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
    "Build\MyNotePad.exe"
)

$exePath = $null
foreach ($path in $possiblePaths) {
    if (Test-Path $path) {
        $exePath = $path
        break
    }
}

if (-not $exePath) {
    Write-Error "Build output MyNotePad.exe not found"
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
Copy-Item $exePath "$distDir\MyNotePad.exe" -Force

$pdbPath = $exePath -replace "\.exe$", ".pdb"
if (Test-Path $pdbPath) {
    Copy-Item $pdbPath "$distDir\MyNotePad.pdb" -Force
    Write-Host "PDB file copied" -ForegroundColor Green
}

Write-Host "Distribution files copied to: $distDir\" -ForegroundColor Green

# Optional: Run tests
if ($Test) {
    Write-Host "Starting application for testing..." -ForegroundColor Yellow
    Start-Process "$distDir\MyNotePad.exe"
}

Write-Host "=== Build Complete ===" -ForegroundColor Green
