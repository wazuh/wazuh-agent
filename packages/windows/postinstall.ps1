$logFile = Join-Path -Path "C:\ProgramData" -ChildPath "postinstall.log"
Start-Transcript -Path $logFile

Write-Host "Running postinstall.ps1"


# Create required directories
$directoriesToCreate = @(
    "C:\ProgramData\wazuh-agent",
    "C:\ProgramData\wazuh-agent\config",
    "C:\ProgramData\wazuh-agent\config\shared",
    "C:\ProgramData\wazuh-agent\var",
    "C:\ProgramData\wazuh-agent\run"
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


# Move configuration file
$sourcePath = "$PSScriptRoot\wazuh-agent.yml"
$destinationPath = "C:\ProgramData\wazuh-agent\config\"
Write-Host "Moving $sourcePath to $destinationPath"
if (Test-Path $sourcePath) {
    Move-Item -Path $sourcePath -Destination $destinationPath -Force
    Write-Host "Config file moved successfully."
} else {
    Write-Host "Source file not found: $sourcePath"
}


# Start Wazuh service
$serviceName = "Wazuh Agent"
Write-Host "Attempting to start service: $serviceName"
if (Get-Service -Name $serviceName -ErrorAction SilentlyContinue) {
    $wazuhagent = "$PSScriptRoot\wazuh-agent.exe"
    & $wazuhagent --install-service
    Write-Host "Service $serviceName started successfully."
} else {
    Write-Host "Service $serviceName not found. Skipping."
}
Write-Host "postinstall.ps1 script completed."
Stop-Transcript
