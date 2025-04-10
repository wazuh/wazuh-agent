$programData = [System.Environment]::GetFolderPath("CommonApplicationData")

## Create required directories
$directoriesToCreate = @(
    "$programData\wazuh-agent",
    "$programData\wazuh-agent\config",
    "$programData\wazuh-agent\config\shared",
    "$programData\wazuh-agent\var",
    "$programData\wazuh-agent\run"
)
foreach ($dir in $directoriesToCreate) {
    if (-not (Test-Path $dir)) {
        Write-Host "Creating directory: $dir"
        New-Item -ItemType Directory -Path $dir | Out-Null
    } else {
        Write-Host "Directory already exists: $dir"
    }
}
Write-Host "Directories created successfully."

## Handle Wazuh config file
$destinationDir = Join-Path -Path $programData -ChildPath "wazuh-agent\config"
$sourcePath = "$PSScriptRoot\wazuh-agent.yml"
$destinationPath = Join-Path -Path $destinationDir -ChildPath "wazuh-agent.yml"
$backupPath = Join-Path -Path $destinationDir -ChildPath "wazuh-agent.yml.save"

## Check if backup file exists and remove it
if (Test-Path $backupPath) {
    Write-Host "Deleting old config save $backupPath"
    Remove-Item -Path $backupPath -Force
}

## Check if configuration file exists and back it up
if (Test-Path $destinationPath) {
    Write-Host "Comparing $destinationPath with $sourcePath"
    if ((Get-FileHash $destinationPath).Hash -ne (Get-FileHash $sourcePath).Hash) {
        Write-Host "Current configuration file differs from the source. Creating a backup as $backupPath."
        Rename-Item -Path $destinationPath -NewName $backupPath -Force
    } else {
        Write-Host "Current configuration file is identical to the source. No backup needed."
        Remove-Item -Path $destinationPath -Force
    }
}

## Move the new configuration file
if (Test-Path $sourcePath) {
    try {
        Move-Item -Path $sourcePath -Destination $destinationPath -Force
        Write-Host "Config file moved successfully."
    } catch {
        Write-Host "Failed to move config file: $_"
        exit 1
    }
} else {
    Write-Host "Source file not found: $sourcePath"
}

## Install Wazuh service
$serviceName = "wazuh-agent"
$wazuhagent = "$PSScriptRoot\wazuh-agent.exe"

Write-Host "Installing service $serviceName."
if (-not (Get-Service -Name $serviceName -ErrorAction SilentlyContinue)) {
    & $wazuhagent --install-service
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Error wazuh-agent installing service. Error code: $LASTEXITCODE."
        exit 1
    }
    Write-Host "Service $serviceName installed successfully."
} else {
    Write-Host "Service $serviceName already installed."
}

## Register "wazuh-agent" as an Event Log source if not already present
$logName = "Application"
$sourceName = "wazuh-agent"

$eventLogExists = [System.Diagnostics.EventLog]::SourceExists($sourceName)

if (-not $eventLogExists) {
    try {
        Write-Host "Registering event log source: $sourceName"
        New-EventLog -LogName $logName -Source $sourceName
        Write-Host "Event log source registered successfully."
    } catch {
        Write-Host "Failed to register event log source: $_"
        exit 1
    }
} else {
    Write-Host "Event log source '$sourceName' is already registered."
}

## Define native methods
Add-Type @"
using System;
using System.Runtime.InteropServices;

public class NativeMethods {
    [DllImport("kernel32.dll", SetLastError = true)]
    public static extern bool Wow64DisableWow64FsRedirection(ref IntPtr oldValue);

    [DllImport("kernel32.dll", SetLastError = true)]
    public static extern bool Wow64RevertWow64FsRedirection(IntPtr oldValue);
}
"@

if ($env:ProgramFiles) {
    $installDir = "$env:ProgramFiles\wazuh-agent"
} else {
    $installDir = "C:\Program Files\wazuh-agent"
}

$batContent = @"
@echo off
"$wazuhagent" %*
"@

## Disable WOW64 redirection
$oldValue = [IntPtr]::Zero
$disableSuccess = [NativeMethods]::Wow64DisableWow64FsRedirection([ref]$oldValue)

try {
    $batContent | Out-File "C:\Windows\System32\wazuh-agent.bat" -Encoding ASCII -Force
    Write-Host "Batch file created at: C:\Windows\System32\wazuh-agent.bat" -ForegroundColor Green
}
catch {
    Write-Host "Error writing batch file: $_" -ForegroundColor Red
}
finally {
    if ($disableSuccess) {
        [NativeMethods]::Wow64RevertWow64FsRedirection($oldValue)
    }
}

Write-Host "postinstall.ps1 script completed."

## Delete script after execution
$scriptPath = "$PSScriptRoot\postinstall.ps1"
Write-Host "Deleting script: $scriptPath"
Remove-Item -Path $scriptPath -Force
