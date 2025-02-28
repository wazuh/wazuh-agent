# Back Up and Restore

This document outlines the procedures for backing up and restoring the Wazuh Agent application across different
operating systems. This may be necessary if you want to start the same agent after restoring an endpoint. However, it is
not recommended for moving an agent from one endpoint to another.

## Backup Procedure

### Linux
1. Stop the Wazuh Agent service:
   ```sh
   sudo systemctl stop wazuh-agent
   ```

2. Preparing the backup

   On the agent machine you're doing the back up for, run the following commands to create the destination folder where to
   store the files. These commands use date and time references for the folder name to keep files separated from old
   backups you might have.

   ```sh
   bkp_folder=~/wazuh_files_backup/$(date +%F_%H-%M-%S)
   mkdir -p $bkp_folder
   ```
3. Back up Wazuh agent data and configuration file:
   ```sh
   sudo rsync -aR /etc/wazuh-agent/wazuh-agent.yml $bkp_folder/
   sudo rsync -aR /var/lib/wazuh-agent/agent_info.db $bkp_folder/
   ```

### macOS

1. Stop the Wazuh Agent service:
   ```sh
   sudo launchctl unload /Library/LaunchDaemons/com.wazuh.agent.plist
   ```

2. Preparing the backup:
   ```sh
   bkp_folder=~/wazuh_files_backup/$(date +%F_%H-%M-%S)
   mkdir -p $bkp_folder
   ```

3. Back up Wazuh agent data and configuration file:
   ```sh
   sudo rsync -aR "/Library/Application Support/Wazuh agent.app/etc/wazuh-agent.yml" $bkp_folder/
   sudo rsync -aR "/Library/Application Support/Wazuh agent.app/var/agent_info.db" $bkp_folder/
   ```

### Windows

1. Stop the Wazuh Agent service:
   ```sh
   net stop "Wazuh Agent"
   ```

2. Preparing the backup:
   ```sh
   $bkp_folder = "$env:USERPROFILE\wazuh_files_backup\$(Get-Date -Format 'yyyy-MM-dd_HH-mm-ss')"
   New-Item -ItemType Directory -Path $bkp_folder -Force | Out-Null
   ```

3. Back up Wazuh agent data and configuration file:
   ```sh
   xcopy "C:\ProgramData\wazuh-agent\config\wazuh-agent.yml" "%bkp_folder%\ProgramData\wazuh-agent\config\" /K /S /X /I
   xcopy "C:\ProgramData\wazuh-agent\var\agent_info.db" "%bkp_folder%\ProgramData\wazuh-agent\var\" /K /S /X /I
   ```

## Restore Procedure

### Linux
1. Navigate to the backup folder:
   ```sh
   cd $bkp_folder
   ```

2. Copy files to their respective locations:
   ```sh
   sudo cp etc/wazuh-agent/wazuh-agent.yml /etc/wazuh-agent/
   sudo cp var/lib/wazuh-agent/agent_info.db /var/lib/wazuh-agent/
   ```

3. Start the service:
   ```sh
   sudo systemctl start wazuh-agent
   ```

### macOS

1. Navigate to the backup folder:
   ```sh
   cd $bkp_folder
   ```

2. Copy files to their respective locations:
   ```sh
   sudo cp "Library/Application Support/Wazuh agent.app/etc/wazuh-agent.yml" "/Library/Application Support/Wazuh agent.app/etc/"
   sudo cp "Library/Application Support/Wazuh agent.app/var/agent_info.db" "/Library/Application Support/Wazuh agent.app/var/"
   ```

3. Start the service:
   ```sh
   sudo launchctl load /Library/LaunchDaemons/com.wazuh.agent.plist
   ```

### Windows

1. Navigate to the backup folder:
   ```sh
   Set-Location -Path "$bkp_folder"
   ```

2. Copy files to their respective locations:
   ```sh
   xcopy "ProgramData\wazuh-agent\config\wazuh-agent.yml" "C:\ProgramData\wazuh-agent\config\" /K /S /X /I
   xcopy "\ProgramData\wazuh-agent\var\agent_info.db" "C:\ProgramData\wazuh-agent\var\" /K /S /X /I
   ```

3. Start the service:
   ```sh
   net start "Wazuh Agent"
   ```
