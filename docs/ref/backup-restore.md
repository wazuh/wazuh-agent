# Back Up and Restore

This document outlines the procedures for backing up and restoring the Wazuh Agent application across different
operating systems.

## Backup Procedure

### Linux
1. Stop the Wazuh Agent service:
   ```sh
   sudo systemctl stop wazuh-agent
   ```
2. Create a backup of the necessary directories:
   ```sh
   sudo tar -czvf wazuh-agent-backup.tar.gz \
       /etc/wazuh-agent \
       /var/lib/wazuh-agent \
       /etc/wazuh-agent/shared
   ```
3. Store the backup file in a safe location.

### macOS
1. Stop the Wazuh Agent service:
   ```sh
   sudo launchctl unload /Library/LaunchDaemons/com.wazuh.agent.plist
   ```
2. Create a backup of the necessary directories:
   ```sh
   sudo tar -czvf wazuh-agent-backup.tar.gz \
       "/Library/Application Support/Wazuh agent.app/etc" \
       "/Library/Application Support/Wazuh agent.app/var" \
       "/Library/Application Support/Wazuh agent.app/etc/shared"
   ```
3. Store the backup file in a safe location.

### Windows
1. Stop the Wazuh Agent service:
   ```powershell
   Stop-Service -Name WazuhAgent
   ```
2. Create a backup of the necessary directories:
   ```powershell
   Compress-Archive -Path "C:\ProgramData\wazuh-agent\config", \
       "C:\ProgramData\wazuh-agent\var", \
       "C:\ProgramData\wazuh-agent\config\shared" \
       -DestinationPath wazuh-agent-backup.zip
   ```
3. Store the backup file in a safe location.

## Restore Procedure

### Linux
1. Extract the backup file:
   ```sh
   sudo tar -xzvf wazuh-agent-backup.tar.gz -C /
   ```
2. Copy extracted files to their respective locations:
   ```sh
   sudo cp -r etc/wazuh-agent /etc/
   sudo cp -r var/lib/wazuh-agent /var/lib/
   sudo cp -r etc/wazuh-agent/shared /etc/wazuh-agent/
   ```
3. Reload the systemd daemon and start the service:
   ```sh
   sudo systemctl daemon-reload
   sudo systemctl start wazuh-agent
   ```

### macOS
1. Extract the backup file:
   ```sh
   sudo tar -xzvf wazuh-agent-backup.tar.gz -C /
   ```
2. Copy extracted files to their respective locations:
   ```sh
   sudo cp -r "Library/Application Support/Wazuh agent.app/etc" "/Library/Application Support/"
   sudo cp -r "Library/Application Support/Wazuh agent.app/var" "/Library/Application Support/"
   sudo cp -r "Library/Application Support/Wazuh agent.app/etc/shared" "/Library/Application Support/Wazuh agent.app/etc/"
   ```
3. Load and start the service:
   ```sh
   sudo launchctl load /Library/LaunchDaemons/com.wazuh.agent.plist
   ```

### Windows
1. Extract the backup file:
   ```powershell
   Expand-Archive -Path wazuh-agent-backup.zip -DestinationPath "C:\"
   ```
2. Copy extracted files to their respective locations:
   ```powershell
   Copy-Item -Path "C:\wazuh-agent-backup\ProgramData\wazuh-agent\config" -Destination "C:\ProgramData\wazuh-agent\" -Recurse -Force
   Copy-Item -Path "C:\wazuh-agent-backup\ProgramData\wazuh-agent\var" -Destination "C:\ProgramData\wazuh-agent\" -Recurse -Force
   Copy-Item -Path "C:\wazuh-agent-backup\ProgramData\wazuh-agent\config\shared" -Destination "C:\ProgramData\wazuh-agent\config\" -Recurse -Force
   ```
3. Start the Wazuh Agent service:
   ```powershell
   Start-Service -Name WazuhAgent
   ```
