# Usage

## Enroll the Agent

To allow the agent to successfully connect to a Wazuh server, it must first enroll with the server. This is done using the --enroll option.
Ensure that the server is online and ready to accept enrollment requests before proceeding.

## Enrollment Command on Linux

```
/usr/share/wazuh-agent/bin/wazuh-agent --enroll --enroll-url https://localhost:55000 --user <username> --password <password> [--name <agent-name>]
```

Replace:

* `<username>` with the Wazuh server username.
* `<password>` with the corresponding password.
* `<agent-name>` with the desired name for the agent (optional).

### View Enrollment Help

For additional options and details, use:

```
/usr/share/wazuh-agent/bin/wazuh-agent --enroll --help
```

## Enrollment Command on MacOS

```
/Library/Application\ Support/Wazuh\ agent.app/bin/wazuh-agent --enroll --enroll-url https://localhost:55000 --user <username> --password <password> [--name <agent-name>]
```

Replace:

* `<username>` with the Wazuh server username.
* `<password>` with the corresponding password.
* `<agent-name>` with the desired name for the agent (optional).

### View Enrollment Help

For additional options and details, use:

```
/Library/Application\ Support/Wazuh\ agent.app/bin/wazuh-agent --enroll --help
```

## Enrollment Command on Windows

```
"C:\Program Files\wazuh-agent\wazuh-agent.exe" --enroll --enroll-url https://localhost:55000 --user <username> --password <password> [--name <agent-name>]
```

Replace:

* `<username>` with the Wazuh server username.
* `<password>` with the corresponding password.
* `<agent-name>` with the desired name for the agent (optional).

### View Enrollment Help

For additional options and details, use:

```
"C:\Program Files\wazuh-agent\wazuh-agent.exe" --enroll --help
```

## Run the Agent on Linux

- **To run the agent in the foreground from the CLI**

    The agent can be started and its status checked with:

    ```bash
    wazuh-agent
    wazuh-agent --status
    ```

- **To run the agent as a systemd service**

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
    wazuh-agent
    wazuh-agent --status
    ```

- **To run the agent as a launchd service**

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
    wazuh-agent
    wazuh-agent --status
    ```

- **To run the agent as a Windows service**

    The agent service can be started, stopped, or restarted from the CLI:

    ```bash
    net start "wazuh-agent"
    net stop "wazuh-agent"
    ```

    > This can also be done using Windows SCM.
