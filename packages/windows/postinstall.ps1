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

# Install Wazuh service
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

# Register "wazuh-agent" as an Event Log source if not already present
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

Write-Host "postinstall.ps1 script completed."

# Delete script after execution
$scriptPath = "$PSScriptRoot\postinstall.ps1"
Write-Host "Deleting script: $scriptPath"
Remove-Item -Path $scriptPath -Force
