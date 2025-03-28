# Run from Sources

## Enroll the Agent

To allow the agent to successfully connect to a Wazuh server, it must first enroll with the server. This is done using the --enroll option.
Ensure that the server is online and ready to accept enrollment requests before proceeding.

### Enrollment Command

```
cd build/
./wazuh-agent --enroll --enroll-url https://localhost:55000 --user <username> --password <password> [--name <agent-name>]
```

Replace:

* <username> with your Wazuh server username.
* <password> with your corresponding password.
* <agent-name> with the desired name for the agent (optional).

### View Enrollment Help

For additional options and details, use:

```
cd build/
./wazuh-agent --enroll --help
```

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
