$CleanPath = "C:\ProgramData\wazuh-agent"
Write-Host "Running cleanup.ps1 for path $CleanPath"

# Remove Wazuh data folder
if (Test-Path $CleanPath) {
    Get-ChildItem -Path $CleanPath -Recurse -File | ForEach-Object {
        try {
            Write-Host "Removing file: $($_.FullName)"
            Remove-Item -Path $_.FullName -Force
        } catch {
            Write-Host "Failed to remove file: $_"
            exit 1
        }
    }

    Get-ChildItem -Path $CleanPath -Recurse -Directory | Sort-Object -Property FullName -Descending | ForEach-Object {
        if (-not (Get-ChildItem -Path $_.FullName -Recurse)) {
            try {
                Write-Host "Removing empty directory: $($_.FullName)"
                Remove-Item -Path $_.FullName -Force
            } catch {
                Write-Host "Failed to remove empty directory: $_"
                exit 1
            }
        }
    }

    if (-not (Get-ChildItem -Path $CleanPath -Recurse)) {
        try {
            Write-Host "Removing root directory: $CleanPath"
            Remove-Item -Path $CleanPath -Force
        } catch {
            Write-Host "Failed to remove root directory: $_"
            exit 1
        }
    }
} else {
    Write-Host "Cleanup path $CleanPath does not exist. Skipping."
}

# Remove Wazuh service
$serviceName = "wazuh-agent"
$wazuhagent = "$PSScriptRoot\wazuh-agent.exe"

Write-Host "Removing service $serviceName."
if (Get-Service -Name $serviceName -ErrorAction SilentlyContinue) {
    & $wazuhagent --remove-service
    Write-Host "Service $serviceName removed successfully."
} else {
    Write-Host "Service $serviceName not found."
}

# Remove Event Log source if it exists
$sourceName = "wazuh-agent"

if ([System.Diagnostics.EventLog]::SourceExists($sourceName)) {
    try {
        Write-Host "Removing event log source: $sourceName"
        Remove-EventLog -Source $sourceName
        Write-Host "Event log source removed successfully."
    } catch {
        Write-Host "Failed to remove event log source: $_"
        exit 1
    }
} else {
    Write-Host "Event log source '$sourceName' has already been removed."
}

Write-Host "cleanup.ps1 script completed."
