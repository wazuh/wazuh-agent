# Created by Wazuh, Inc. <info@wazuh.com>.
# This program is a free software; you can redistribute it and/or modify it under the terms of GPLv2

param (
    [string]$MSI_NAME = "wazuh-agent",
    [string]$BUILD_TESTS = "0",
    [string]$CMAKE_CONFIG = "Debug",
    [string]$TOKEN_VCPKG = "",
    [int]$JOBS = 0
)

if(($help.isPresent)) {
    "
    This tool can be used to generate the Windows Wazuh agent msi package.

    PARAMETERS TO BUILD WAZUH-AGENT MSI (OPTIONALS):
        1. MSI_NAME: MSI package name output. By default 'wazuh-agent'.
        2. BUILD_TESTS: Define test mode action (0 or 1). By default '0'.
        3. CMAKE_CONFIG: Cmake config type, Debug, Release, RelWithDebInfo or MinSizeRel. By default 'Debug'.
        4. TOKEN_VCPKG: VCPKG remote binary caching repository key. By default is empty, no binary caching funcionality will be used.
        5. JOBS: Number of threads to use for compilation. If 0, it will use all logical processors.
    "
    Exit
}
function Set-VcpkgRemoteBinaryCache {
    param(
        [Parameter(Mandatory=$true)]
        [string]$VcpkgToken
    )

    Write-Host "Enabling VCPKG remote binary caching for nuget..."

    $env:VCPKG_BINARY_SOURCES = "clear;nuget,GitHub,readwrite"
    $nugetPath = "C:\nuget\nuget.exe"

    if (!(Test-Path $nugetPath)) {
        mkdir "C:\nuget" -Force
        Write-Host "Downloading nuget.exe to $nugetPath..."
        Invoke-WebRequest -Uri "https://dist.nuget.org/win-x86-commandline/v6.10.2/nuget.exe" -OutFile $nugetPath
    }

    & $nugetPath sources add `
        -source "https://nuget.pkg.github.com/wazuh/index.json" `
        -name "GitHub" `
        -username "wazuh" `
        -password $VcpkgToken

    & $nugetPath setapikey $VcpkgToken `
        -source "https://nuget.pkg.github.com/wazuh/index.json"

    Write-Host "VCPKG remote binary cache has been configured."
}

if ($TOKEN_VCPKG -ne "") {
    Set-VcpkgRemoteBinaryCache -VcpkgToken $TOKEN_VCPKG
} else {
    Write-Host "Skipping VCPKG remote binary caching: TOKEN_VCPKG is empty."
}

$originalDir = Get-Location
cd $PSScriptRoot/../..
mkdir build -Force
$Env:CUSTOM_PACKAGE_NAME = $MSI_NAME
$Env:CUSTOM_CMAKE_CONFIG = $CMAKE_CONFIG
cmake src -B build `
  -DBUILD_TESTS=$BUILD_TESTS `
  -G "Visual Studio 16 2019" `
  -A x64 `
  -T v141 `
  -DCMAKE_SYSTEM_VERSION=6.1 `
  -D_WIN32_WINNT=0x0601

if ($JOBS -le 0) {
    $JOBS = (Get-CimInstance Win32_ComputerSystem).NumberOfLogicalProcessors
    Write-Host "JOBS not specified or <= 0. Using $JOBS logical processors."
} else {
    Write-Host "Using $JOBS threads for compilation."
}

cmake --build build --config $CMAKE_CONFIG --parallel $JOBS
cd $originalDir
