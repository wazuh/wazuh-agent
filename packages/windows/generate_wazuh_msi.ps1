# Created by Wazuh, Inc. <info@wazuh.com>.
# This program is a free software; you can redistribute it and/or modify it under the terms of GPLv2

param (
    [string]$MSI_NAME = "wazuh-agent",
    [string]$SIGN = "no",
    [string]$DEBUG = "no",
    [string]$CMAKE_CONFIG = "Debug",
    [string]$SIGN_TOOLS_PATH = "",
    [string]$CV2PDB_PATH = "",
    [string]$CERTIFICATE_PATH = "",
    [string]$CERTIFICATE_PASSWORD = "",
    [switch]$help
)

$SIGNTOOL_EXE = "signtool.exe"
$CV2PDB_EXE = "cv2pdb.exe"

if(($help.isPresent)) {
    "
    This tool can be used to generate the Windows Wazuh agent msi package.

    PARAMETERS TO BUILD WAZUH-AGENT MSI (OPTIONALS):
        1. MSI_NAME: MSI package name output. By default 'wazuh-agent'.
        2. SIGN: Define sign action process (yes or no). By default 'no'.
        3. DEBUG: Define debug symbols generation process (yes or no). By default 'no'.
        4. CMAKE_CONFIG: Cmake config type, Debug, Release, RelWithDebInfo or MinSizeRel. By default 'Debug'.
        5. SIGN_TOOLS_PATH: Sign tools path. Not needed if PATH env var is correctly configured.
        6. CV2PDB_PATH: Debug symbols tools path. Not needed if PATH env var is correctly configured.
        7. CERTIFICATE_PATH: Path to the .pfx certificate file. If not specified, signtool /a parameter will be use.
        8. CERTIFICATE_PASSWORD: Password for the .pfx certificate file. If not specified, signtool /a parameter will be use.

    USAGE:

        * WAZUH:
          $ ./generate_wazuh_msi.ps1  -MSI_NAME {{ NAME }} -SIGN {{ yes|no }} -DEBUG {{ yes|no }} -SIGN_TOOLS_PATH {{ PATH }} -CV2PDB_PATH {{ PATH }} -CERTIFICATE_PATH {{ CERTIFICATE_PATH }} -CERTIFICATE_PASSWORD {{ CERTIFICATE_PASSWORD }}
            Build a devel msi:    $ ./generate_wazuh_msi.ps1 -MSI_NAME wazuh-agent_4.9.0-0_windows_0ceb378 -SIGN no
            Build a prod msi:     $ ./generate_wazuh_msi.ps1 -MSI_NAME wazuh-agent-4.9.0-1 -SIGN yes
    "
    Exit
}

function BuildWazuhMsi(){

    $MSI_NAME = "$MSI_NAME.msi"
    Write-Host "MSI_NAME = $MSI_NAME"

    if($DEBUG -eq "yes"){
        if($CV2PDB_PATH -ne ""){
            $CV2PDB_EXE = $CV2PDB_PATH + "/" + $CV2PDB_EXE
        }

        $originalDir = Get-Location
        cd $PSScriptRoot/../../build/$CMAKE_CONFIG

        $exeFiles = @()

        # Every .exe
        $exeFiles += Get-ChildItem -Filter "*.exe"

        # Extract symbols
        foreach ($file in $exeFiles)
        {
            Write-Host "Extracting dbg symbols from" $file.FullName
            $args = $file.FullName # Source (exe with debug symbols)
            $args += " "
            $args += $file.FullName  # Destination (same as source - exe is stripped of debug symbols)
            $args += " "
            $args += $file.BaseName
            $args += ".pdb"

            Start-Process -FilePath $CV2PDB_EXE -ArgumentList $args -WindowStyle Hidden
        }

        Write-Host "Waiting for processes to finish"
        Wait-Process -Name cv2pdb -Timeout 10

        # Compress every pdb file in current folder
        $pdbFiles = Get-ChildItem -Filter "*.pdb"

        $ZIP_NAME = "$($MSI_NAME.Replace('.msi', '-debug-symbols.zip'))"

        Write-Host "Compressing debug symbols to $ZIP_NAME"
        Compress-Archive -Path $pdbFiles -Force -DestinationPath "$ZIP_NAME"

        dir "*debug-symbols.zip"

        Remove-Item -Path "*.pdb"

        cd $originalDir

    }

    if($SIGN_TOOLS_PATH -ne ""){
        $SIGNTOOL_EXE = $SIGN_TOOLS_PATH + "/" + $SIGNTOOL_EXE
    }

    if($SIGN -eq "yes"){
        # Determine signing command options
        $signOptions = @()
        if ($CERTIFICATE_PATH -ne "" -and $CERTIFICATE_PASSWORD -ne "") {
            $signOptions += "/f"
            $signOptions += "`"$CERTIFICATE_PATH`""
            $signOptions += "/p"
            $signOptions += "`"$CERTIFICATE_PASSWORD`""
        } else {
            $signOptions += "/a"
        }

        # Define files to sign
        $filesToSign = @(
            "$PSScriptRoot\..\..\build\$CMAKE_CONFIG\*.exe",
            "$PSScriptRoot\postinstall.ps1",
            "$PSScriptRoot\cleanup.ps1"
        )

        # Sign the files
        foreach ($file in $filesToSign) {
            Write-Host "Signing $file with"
            & $SIGNTOOL_EXE sign $signOptions /tr http://timestamp.digicert.com /fd SHA256 /td SHA256 "$file"
        }
    }

    Write-Host "Building MSI installer..."

    cpack -C $CMAKE_CONFIG --config $PSScriptRoot\..\..\build\CPackConfig.cmake -B "$PSScriptRoot"

    if($SIGN -eq "yes"){
        Write-Host "Signing $MSI_NAME..."
        & $SIGNTOOL_EXE sign $signOptions /tr http://timestamp.digicert.com /d $MSI_NAME /fd SHA256 /td SHA256 "$PSScriptRoot\$MSI_NAME"
    }
}

############################
# MAIN
############################

BuildWazuhMsi
