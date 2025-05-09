# Run from Sources

To build the agent from source, refer to: [build-sources.md](./build-sources.md).

> Note: After building, the agent executables are located in:
> - **Linux/macOS:** `wazuh-agent/build`
> - **Windows:** `wazuh-agent\build\Debug`

## Enroll the Agent

To allow the agent to successfully connect to a Wazuh server, it must first enroll with the server. This is done using the --enroll option.
Ensure that the server is online and ready to accept enrollment requests before proceeding.

### Enrollment Command

- **Linux/macOS:**
```
cd build/
./wazuh-agent --enroll --enroll-url https://localhost:55000 --user <username> --password <password> [--name <agent-name>]
```

- **Windows:**
```
cd .\build\Debug\
./wazuh-agent --enroll --enroll-url https://localhost:55000 --user <username> --password <password> [--name <agent-name>]
```

Replace:

* `<username>` with the Wazuh server username.
* `<password>` with the corresponding password.
* `<agent-name>` with the desired name for the agent (optional).

### View Enrollment Help

For additional options and details, use:

```
./wazuh-agent --enroll --help
```

### Important Notes About Enrollment

The agent stores information in a local database during enrollment. By default, the database is created in the following paths:

- **Linux:** `/var/lib/wazuh-agent`
- **macOS:** `/Library/Application Support/Wazuh agent.app/var`
- **Windows:** `C:\\ProgramData\\wazuh-agent\\var`

If these directories do not exist at the time of running the enrollment command **when running from sources**, the agent will fail with an error like:

```
Error: Cannot open database: C:\ProgramData\wazuh-agent\var/agent_info.db
```

To avoid this, **manually create the directory** before running the command, depending on the operating system:

- **Linux:**
  ```bash
  sudo mkdir -p /var/lib/wazuh-agent
  ```

- **macOS:**
  ```bash
  sudo mkdir -p "/Library/Application Support/Wazuh agent.app/var"
  ```

- **Windows:**
  ```powershell
  mkdir "C:\ProgramData\wazuh-agent\var"
  ```

> Note: If the agent is installed using the installation process, the necessary directories are automatically created.

**Alternative: Use a Custom Data Path**

A custom location for the agent data can be configured by setting the `path.data` option in the `wazuh-agent.yml` configuration file:

```yaml
agent:
  ...
  path.data: "C:/MyCustomPath/"
```

> Note: Setting `path.data: "."` will make the agent use the current working directory as the data directory.

Default configuration file locations:

- **Linux:** `/etc/wazuh-agent/wazuh-agent.yml`
- **macOS:** `/Library/Application Support/Wazuh agent.app/etc/wazuh-agent.yml`
- **Windows:** `C:\\Program Files\\wazuh-agent\\config\\wazuh-agent.yml`

### Use a Custom Configuration File

A custom config file can also be specified during enrollment using the `--config-file` option:

```bash
./wazuh-agent --enroll --enroll-url https://localhost:55000 --user <username> --password <password> [--name <agent-name>] --config-file /path/to/wazuh-agent.yml
```

This is useful when testing with different configurations or running the agent from a non-default directory.

## Run the Agent on Linux

- **To run the agent in the foreground from the CLI**

    The agent can be started and its status checked with:

    ```bash
    cd build/
    ./wazuh-agent
    ./wazuh-agent --status
    ```

- **To run the agent as a systemd service**

    Install the agent:

    ```bash
    cmake --install build --prefix /
    ```

    > Note: The agent can be installed in any directory, as long as the `path.data` parameter in `wazuh-config.yml` is configured correctly. For more information, see [configuration.md](../ref/configuration.md#agent).

    Enable the service:

    ```bash
    systemctl enable wazuh-agent
    ```

    The agent can be started, stopped, and its status checked using systemctl:

    ```bash
    systemctl start wazuh-agent
    systemctl stop wazuh-agent
    systemctl is-active wazuh-agent
    systemctl status wazuh-agent
    ```

## Run the Agent on macOS

- **To run the agent in the foreground from the CLI**

    The agent can be started and its status checked with:

    ```bash
    cd build/
    ./wazuh-agent
    ./wazuh-agent --status
    ```

- **To run the agent as a launchd service**

    Install the agent:

    ```bash
    sudo cmake --install build --prefix /
    ```

    > Note: The agent can be installed in any directory, as long as the `path.data` parameter in `wazuh-config.yml` is configured correctly. For more information, see [configuration.md](../ref/configuration.md#agent).

    Load the service:

    ```bash
    sudo launchctl bootstrap system /Library/LaunchDaemons/com.wazuh.agent.plist
    ```

    > This command has superseeded `load` in the legacy syntax. The daemon will run after load as indicated
    in the property list file.

    Unload the service:

    ```bash
    sudo launchctl bootout system /Library/LaunchDaemons/com.wazuh.agent.plist
    ```

    > This command has superseeded `unload` in the legacy syntax.

    Verify the service is running:

    ```bash
    sudo launchctl print system/com.wazuh.agent
    ```

## Run the Agent on Windows

- **To run the agent in the foreground from the CLI**

    The agent can be started and its status checked with:

    ```bash
    .\wazuh-agent
    .\wazuh-agent --status
    ```

- **To run the agent as a Windows service**

    ```bash
    .\wazuh-agent --install-service
    ```

    The agent service can be started, stopped, or restarted from the CLI:

    ```bash
    net start "wazuh-agent"
    net stop "wazuh-agent"
    ```

    > This can also be done using Windows SCM.

    To remove the service:

    ```bash
    .\wazuh-agent --remove-service
    ```
