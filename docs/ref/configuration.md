# Configuration

## Environment variables

The only available environment variable is `SPDLOG_LEVEL`. This variable controls the logging level of the application.

### Possible values
The `SPDLOG_LEVEL` variable can be set to any of the following values, corresponding to the log levels in `spdlog`:

- `trace`
- `debug`
- `info`
- `warn`
- `err`
- `critical`

### Example usage

To run `wazuh-agent` with a specific log level, use the following command:

  ```bash
  sudo SPDLOG_LEVEL=debug ./wazuh-agent
  ```

This will start the `wazuh-agent` process with `debug` level logging, allowing for detailed debugging output.

## Command line options

| Option                | Description                                                                                            | Default |
| --------------------- | ------------------------------------------------------------------------------------------------------ | ------- |
| `--help`              | Display help menu                                                                                      | N/A     |
| `--run`               | Run agent in foreground (this is the default behavior)                                                 | N/A     |
| `--status`            | Check if the agent is running (running or stopped)                                                     | N/A     |
| `--config-file`       | Path to the Wazuh configuration file (optional)                                                        | N/A     |
| `--reload-config`     | Reload configuration file and all modules                                                              | N/A     |
| `--reload-module`     | Reload a specific module by name                                                                       | N/A     |
| `--enroll`            | Use this option to enroll as a new agent                                                               | N/A     |
| `--enroll-url`        | URL of the server management API enrollment endpoint                                                   | N/A     |
| `--user`              | User to authenticate with the server management API                                                    | N/A     |
| `--password`          | Password to authenticate with the server management API                                                | N/A     |
| `--key`               | Key to enroll the agent (optional)                                                                     | N/A     |
| `--name`              | Name to enroll the agent (optional)                                                                    | N/A     |
| `--connect-url`       | URL of the server (optional)                                                                           | N/A     |
| `--verification-mode` | Verification mode to be applied on HTTPS connection to the server (full, certificate, none) (optional) | N/A     |

## Configuration file

The `wazuh-agent.yml` configuration file contains the following sections that can be configured:

### Agent

```yaml
agent:
  thread_count: 4
  server_url: https://localhost:27000
  retry_interval: 30s
  verification_mode: none
  path.data: "/var/lib/wazuh-agent"
  path.run: "/var/run"
  queue_size: 10000
```

| Mandatory | Option              | Description                                                       | Default                   |
| :-------: | ------------------- | ----------------------------------------------------------------- | ------------------------- |
|           | `thread_count`      | Number of worker threads                                          | 4                         |
|           | `server_url`        | URL of the server                                                 | `https://localhost:27000` |
|           | `retry_interval`    | Interval to retry connection                                      | 30s                       |
|           | `verification_mode` | Verification mode for HTTPS connections (full, certificate, none) | none                      |
|           | `path.data`         | Path to store agent data                                          | `/var/lib/wazuh-agent`    |
|           | `path.run`          | Path to store runtime files                                       | `/var/run`                |
|           | `queue_size`        | Size of the event queue (min: 1000, max: 3600000)                 | 10000                     |

### Events

```yaml
events:
  batch_interval: 10s
  batch_size: 1MB
```
| Mandatory | Option           | Description                                    | Default |
| :-------: | ---------------- | ---------------------------------------------- | ------- |
|           | `batch_interval` | Agent batch interval (min: 1000, max: 3600000) | 10s     |
|           | `batch_size`     | Agent batch size (min: 1000B, max: 100000000B) | 1MB     |

### Logcollector Module

```yaml
logcollector:
  enabled: true
  reload_interval: 1m
  read_interval: 500ms
  localfiles:
    - location: /var/log/*.log
  journald:
    - field: "_SYSTEMD_UNIT"
      value: "cron.service"
      exact_match: true
      ignore_if_missing: true
  windows:
    - channel: Application
      query: Event[System/EventID = 4624]
  macos:
    - query: process == "sshd" OR message CONTAINS "invalid"
      level: info
      type: trace,activity,log
```

#### Global Configuration

| Mandatory | Option            | Description                                        | Default |
| :-------: | ----------------- | -------------------------------------------------- | ------- |
|           | `enabled`         | Sets the module as enabled                         | true    |
|           | `reload_interval` | Interval to reload configuration                   | 1m      |
|           | `read_interval`   | Interval to read logs                              | 500ms   |
|           | `localfiles`      | Configuration related to local file log readers    | N/A     |
|           | `journald`        | Configuration related to journald log readers      | N/A     |
|           | `windows`         | Configuration related to Windows event log readers | N/A     |
|           | `macos`           | Configuration related to macOS log readers         | N/A     |

#### Localfiles Configuration

| Mandatory | Option     | Description             | Default |
| :-------: | ---------- | ----------------------- | ------- |
|     ✅     | `location` | Path to local log files | N/A     |

#### Journald Configuration

| Mandatory | Option              | Description                      | Default |
| :-------: | ------------------- | -------------------------------- | ------- |
|     ✅     | `field`             | Journald field to filter         | N/A     |
|     ✅     | `value`             | Expected value for the field     | N/A     |
|           | `exact_match`       | Whether the match must be exact  | true    |
|           | `ignore_if_missing` | Ignore entry if field is missing | false   |

#### Windows Configuration

| Mandatory | Option    | Description               | Default |
| :-------: | --------- | ------------------------- | ------- |
|     ✅     | `channel` | Windows event log channel | N/A     |
|     ✅     | `query`   | Windows event log query   | N/A     |

#### macOS Configuration

| Mandatory | Option  | Description                                         | Default |
| :-------: | ------- | --------------------------------------------------- | ------- |
|     ✅     | `query` | macOS log query                                     | N/A     |
|     ✅     | `level` | macOS log level (Debug, Info, Notice, Error, Fault) | N/A     |
|     ✅     | `type`  | macOS log types (trace, activity, log)              | N/A     |

### Inventory Module

```yaml
inventory:
  enabled: true
  interval: 1h
  scan_on_start: true
  hardware: true
  system: true
  networks: true
  packages: true
  ports: true
  ports_all: false
  processes: false
  hotfixes: true
```

| Mandatory | Option          | Description                                                                             | Default |
| :-------: | --------------- | --------------------------------------------------------------------------------------- | ------- |
|           | `enabled`       | Sets the module as enabled                                                              | true    |
|           | `interval`      | Specifies the time between system scans                                                 | 1h      |
|           | `scan_on_start` | Initiates a system scan immediately after start the wazuh-agent service on the endpoint | true    |
|           | `hardware`      | Enables the hardware scan                                                               | true    |
|           | `system`        | Enables the system scan                                                                 | true    |
|           | `networks`      | Enables the network scan                                                                | true    |
|           | `packages`      | Enables the package scan                                                                | true    |
|           | `ports`         | Enables the port scan                                                                   | true    |
|           | `ports_all`     | Enables the all ports scan or only listening ports                                      | false   |
|           | `processes`     | Enables the process scan                                                                | false   |
|           | `hotfixes`      | Enables the hotfix scan                                                                 | true    |

### SCA Module

```yaml
sca:
  enabled: true
  scan_on_start: true
  interval: 1h
  policies:
    - etc/shared/cis_debian10.yml
    - /my/custom/policy/path/my_policy.yaml
  policies_disabled:
    - ruleset/sca/cis_debian9.yml
```

| Mandatory | Option              | Description                                                                 | Default |
| :-------: | ------------------- | --------------------------------------------------------------------------- | ------- |
|           | `enabled`           | Enables or disables the SCA module                                          | yes     |
|           | `scan_on_start`     | Runs an assessment as soon as the agent starts                              | true    |
|           | `interval`          | Time between scans (supports `s`, `m`, `h`, `d`)                            | 1h      |
|           | `policies`          | List of enabled policy file paths                                           | —       |
|           | `policies_disabled` | List of policy file paths to explicitly disable                             | —       |