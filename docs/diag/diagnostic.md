# Diagnostic Guide

1. **Agent version and service status**
2. **Configuration files**
3. **Logs**
4. **System resource usage**

### Agent Status

#### Linux

```
systemctl status wazuh-agent
```

#### macOS

```
sudo launchctl print system/com.wazuh.agent
```

#### Windows

```
Get-Service -Name wazuh-agent
```

### Configuration Files

#### Linux

To gather the configuration file, locate the following file:

- `/etc/wazuh-agent/wazuh-agent.yml`

Copy this file for analysis.

####  macOS

To gather the configuration file, locate the following file:

- `/Library/Application Support/Wazuh agent.app/etc/wazuh-agent.yml`

Copy this file for analysis.

#### Windows

To gather the configuration file, locate the following file:

- `C:\ProgramData\wazuh-agent\config\wazuh-agent.yml`

Copy this file for analysis.

### Logs

#### Linux

Use `journald` or `systemctl status wazuh-agent` to capture logs related to `wazuh-agent`.

```bash
journalctl -u wazuh-agent.service
```

#### macOS

Use the Unified Logging System (ULS) to extract logs related to `wazuh-agent`.

```bash
log show --predicate 'process == "wazuh-agent"' --info
```

#### Windows

Use the Event Viewer to search for logs related to `wazuh-agent`.

### System Resources

#### Linux & macOS

Collect system resource usage data to understand the agent's performance impact. Fill in the table below with the relevant data:

| Daemon               | CPU usage | RAM usage | Disk usage | Network usage               |
|----------------------|-----------|-----------|------------|-----------------------------|
| (Global)             |           |           |            | (Connections to port 27000) |
| `wazuh-agent`        |           |           |            |                             |

#### Steps to collect system resource data

1. **CPU and RAM usage:**
   - Use the `top` or `htop` command to monitor CPU and RAM usage for `wazuh-agent`.
2. **Disk usage:**
   - Use the `df -h` command to check the disk usage of the file system where Wazuh is installed.
   - Use the `du -h` command to check the disk usage of the Wazuh agent installation (usually _/var/lib/wazuh-agent_).
3. **Network usage:**
   - Use the `netstat` or `ss` command to monitor connections to ports 55000 and 27000.
   - Alternatively, use tools like `iftop` or `nload` to measure real-time network usage.

##### Example commands

1. **CPU and RAM usage:**
   ```bash
   top -p $(pgrep -d',' wazuh-agent)
   ```
2. **Disk usage:**
   ```bash
   df -h /usr/share/wazuh-agent
   df -h /var/lib/wazuh-agent
   ```
3. **Network usage:**
   ```bash
   netstat -an | grep ':55000\|:27000'
   ```

#### Windows

Collect system resource usage data to understand the agent's performance impact. Fill in the table below with the relevant data:

| Daemon            | CPU usage | RAM usage | Disk usage | Network usage                     |
|-------------------|-----------|-----------|------------|-----------------------------------|
| `wazuh-agent.exe` |           |           |            |                                   |

#### Steps to collect system resource data

1. **CPU and RAM usage:**
   - Use the Task Manager or the `tasklist` command to monitor CPU and RAM usage for `wazuh-agent.exe`.
2. **Disk usage:**
   - Use the `dir` command to check the disk usage of the directory where Wazuh is installed.
   - Alternatively, use the Disk Management tool or the `Get-PSDrive` PowerShell cmdlet.
3. **Network usage:**
   - Use the `netstat` command to monitor connections to ports 55000 and 27000.
   - Alternatively, use tools like Resource Monitor or PowerShell cmdlets such as `Get-NetTCPConnection`.

##### Example commands

1. **CPU and RAM usage:**
   - Open Task Manager, go to the "Details" tab, and find `wazuh-agent.exe`.
   - Or use the command prompt:
     ```cmd
     tasklist /fi "imagename eq wazuh-agent.exe"
     ```
2. **Disk usage:**
   - Command prompt:
     ```cmd
     dir "C:\ProgramData\wazuh-agent\" /s
     ```
   - PowerShell:
     ```powershell
     Get-PSDrive -PSProvider FileSystem
     ```
3. **Network usage:**
   - Command prompt:
     ```cmd
     netstat -an | findstr ":55000" | findstr ":27000"
     ```
   - PowerShell:
     ```powershell
     Get-NetTCPConnection -LocalPort 55000,27000
     ```
