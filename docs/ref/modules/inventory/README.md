# Inventory Module

## Introduction

The **Inventory** module is responsible for collecting and storing detailed information about the system's configuration, hardware, installed software packages, running processes, active network ports, and network interfaces. The data is structured into multiple tables, making it easy to query and analyze.

## Configuration

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
|           | `enabled`       | Sets the module as enabled                                                              | yes     |
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
