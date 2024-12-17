# Created by Wazuh, Inc. <info@wazuh.com>.
# This program is a free software; you can redistribute it and/or modify it under the terms of GPLv2

param (
    [string]$MSI_NAME = "wazuh-agent",
    [string]$BUILD_TESTS = "0",
    [string]$CMAKE_CONFIG = "Debug"
)

if(($help.isPresent)) {
    "
    This tool can be used to generate the Windows Wazuh agent msi package.

    PARAMETERS TO BUILD WAZUH-AGENT MSI (OPTIONALS):
        1. MSI_NAME: MSI package name output. By default 'wazuh-agent'.
        2. BUILD_TESTS: Define test mode action (0 or 1). By default '0'.
        2. CMAKE_CONFIG: Cmake config type, Debug, Release, RelWithDebInfo or MinSizeRel. By default 'Debug'.
    "
    Exit
}

$originalDir = Get-Location
cd $PSScriptRoot/../..
mkdir build -Force
$Env:CUSTOM_PACKAGE_NAME = $MSI_NAME
$Env:CUSTOM_CMAKE_CONFIG = $CMAKE_CONFIG
cmake src -B build -DBUILD_TESTS=$BUILD_TESTS -G "Visual Studio 17 2022" -A x64
cmake --build build --config $CMAKE_CONFIG
cd $originalDir
