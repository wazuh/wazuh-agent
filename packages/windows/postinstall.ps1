$programData = [System.Environment]::GetFolderPath("CommonApplicationData")

# Create required directories
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


# Handle Wazuh config file
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
    Move-Item -Path $sourcePath -Destination $destinationPath -Force
    Write-Host "Config file moved successfully."
} else {
    Write-Host "Source file not found: $sourcePath"
}


# Install Wazuh service
$serviceName = "Wazuh Agent"
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

Write-Host "postinstall.ps1 script completed."
