$CleanPath = "C:\ProgramData\wazuh-agent"
Write-Host "Running cleanup.ps1 for path $CleanPath"

# List of files to keep after uninstallation
$Exceptions = @(
    "config/wazuh-agent.yml"
)

$ExceptionPaths = $Exceptions | ForEach-Object { Join-Path -Path $CleanPath -ChildPath $_ }

# Remove Wazuh data folder
Get-ChildItem -Path $CleanPath -Recurse -File | ForEach-Object {
    if ($_.FullName -notin $ExceptionPaths) {
        Write-Host "Removing file: $($_.FullName)"
        Remove-Item -Path $_.FullName -Force
    } else {
        Write-Host "Skipping file (exception): $($_.FullName)"
    }
}

Get-ChildItem -Path $CleanPath -Recurse -Directory | Sort-Object -Property FullName -Descending | ForEach-Object {
    if (-not (Get-ChildItem -Path $_.FullName -Recurse)) {
        Write-Host "Removing empty directory: $($_.FullName)"
        Remove-Item -Path $_.FullName -Force
    }
}

if (-not (Get-ChildItem -Path $CleanPath -Recurse)) {
    Write-Host "Removing root directory: $CleanPath"
    Remove-Item -Path $CleanPath -Force
}


# Remove Wazuh service
$serviceName = "Wazuh Agent"
$wazuhagent = "$PSScriptRoot\wazuh-agent.exe"

Write-Host "Removing service $serviceName."
if (Get-Service -Name $serviceName -ErrorAction SilentlyContinue) {
    & $wazuhagent --remove-service
    Write-Host "Service $serviceName removed successfully."
} else {
    Write-Host "Service $serviceName not found."
}

Write-Host "cleanup.ps1 script completed."
