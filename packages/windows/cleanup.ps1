$logFile = Join-Path -Path "C:\ProgramData" -ChildPath "cleanup.log"
Start-Transcript -Path $logFile

$CleanPath = "C:\ProgramData\wazuh-agent"
Write-Host "Running cleanup.ps1 for path $CleanPath"

$Exceptions = @(
    "config/wazuh-agent.yml"
)

$ExceptionPaths = $Exceptions | ForEach-Object { Join-Path -Path $CleanPath -ChildPath $_ }

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

Write-Host "cleanup.ps1 script completed."
Stop-Transcript
