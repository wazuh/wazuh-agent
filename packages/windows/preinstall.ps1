$ConfigPath = "$env:ProgramFiles\wazuh-agent\config"
$ConfigFile = "wazuh-agent.yml"

$FullPath = Join-Path $ConfigPath $ConfigFile
$BackupPath = "$FullPath.save"

## Check if backup file exists and remove it
if (Test-Path $BackupPath) {
    Write-Host "Deleting old config save $BackupPath"
    Remove-Item -Path $BackupPath -Force
}

## Check if configuration file exists and back it up
if (Test-Path $FullPath) {
    Write-Host "Existing config found. Backing up to $BackupPath"
    Move-Item -Path $FullPath -Destination $BackupPath -Force
}

Write-Host "preinstall.ps1 script completed."

# Delete script after execution
$ScriptPath = "$PSScriptRoot\preinstall.ps1"
Write-Host "Deleting script: $ScriptPath"
Remove-Item -Path $ScriptPath -Force
