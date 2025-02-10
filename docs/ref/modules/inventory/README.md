# Inventory Module

## Introduction

The **Inventory** module is responsible for collecting and storing detailed information about the system's configuration, hardware, installed software packages, running processes, active network ports, and network interfaces. The data is structured into multiple tables, making it easy to query and analyze.

---
## Reference
| Mandatory | Option          | Description                                        | Default |
| :-------: | ----------------| -------------------------------------------------- | ------- |
|           | `enabled`       | Sets the module as enabled                         | yes     |
|           | `interval`      | Specifies the time between system scans            | 1h      |
|           | `scan_on_start` | Initiates a system scan immediately after restart the wazuh-agent service on the endpoint | true      |
|           | `hardware`      | Enables the hardware scan                          | true    |
|           | `system`        | Enables the system scan                            | true    |
|           | `networks`      | Enables the network scan                           | true    |
|           | `packages`      | Enables the package scan                           | true    |
|           | `ports`         | Enables the port scan                              | true    |
|           | `ports_all`     | Enables the all ports scan or only listening ports | false   |
|           | `processes`     | Enables the process scan                           | false   |
|           | `hotfixes`      | Enables the hotfix scan                            | true    |


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
---
## Tables

### System Table

```sql
CREATE TABLE system (
    hostname TEXT,
    architecture TEXT,
    os_name TEXT,
    os_version TEXT,
    os_codename TEXT,
    os_build TEXT,
    os_platform TEXT,
    sysname TEXT,
    PRIMARY KEY (os_name)
) WITHOUT ROWID;
```

This table stores information about the operating system and system architecture.

| Mandatory | Column        | Type   | Description                  | Default |
| :-------: | ------------- | ------ | ---------------------------- | ------- |
|           | `hostname`    | TEXT   | Hostname of the machine.     |         |
|           | `architecture`| TEXT   | System architecture.         |         |
|     ✔️     | `os_name`     | TEXT   | Name of the operating system.|         |
|           | `os_version`  | TEXT   | Version of the operating system. |     |
|           | `os_codename` | TEXT   | Codename of the operating system. |   |
|           | `os_build`    | TEXT   | OS build identifier.         |         |
|           | `os_platform` | TEXT   | Platform type (e.g., Linux). |         |
|           | `sysname`     | TEXT   | System name.                 |         |

### Hardware Table

```sql
CREATE TABLE hardware (
    board_serial TEXT,
    cpu_name TEXT,
    cpu_cores INTEGER,
    cpu_mhz INTEGER,
    ram_total INTEGER,
    ram_free INTEGER,
    ram_usage INTEGER,
    PRIMARY KEY (board_serial)
) WITHOUT ROWID;
```

This table stores information about the system hardware.

| Mandatory | Column         | Type    | Description                     | Default |
| :-------: | -------------- | ------- | ------------------------------- | ------- |
|     ✔️     | `board_serial` | TEXT    | Serial number of the motherboard. |         |
|           | `cpu_name`     | TEXT    | Name of the CPU.                |         |
|           | `cpu_cores`    | INTEGER | Number of CPU cores.            |         |
|           | `cpu_mhz`      | INTEGER | CPU speed in MHz.               |         |
|           | `ram_total`    | INTEGER | Total RAM in bytes.             |         |
|           | `ram_free`     | INTEGER | Free RAM in bytes.              |         |
|           | `ram_usage`    | INTEGER | RAM usage in bytes.             |         |

### Hotfixes Table

```sql
CREATE TABLE hotfixes (
    hotfix TEXT,
    PRIMARY KEY (hotfix)
) WITHOUT ROWID;
```

This table stores information about system hotfixes.

| Mandatory | Column   | Type | Description         | Default |
| :-------: | -------- | ---- | ------------------- | ------- |
|     ✔️     | `hotfix` | TEXT | Hotfix identifier.  |         |

### Packages Table

```sql
CREATE TABLE packages (
    name TEXT,
    version TEXT,
    install_time TEXT,
    location TEXT,
    architecture TEXT,
    description TEXT,
    size BIGINT,
    format TEXT,
    PRIMARY KEY (name, version, architecture, format, location)
) WITHOUT ROWID;
```

This table stores information about installed software packages.

| Mandatory | Column         | Type    | Description                              | Default |
| :-------: | -------------- | ------- | ---------------------------------------- | ------- |
|     ✔️     | `name`         | TEXT    | Name of the package.                    |         |
|     ✔️     | `version`      | TEXT    | Version of the package.                 |         |
|           | `install_time` | TEXT    | Installation timestamp.                 |         |
|     ✔️     | `location`     | TEXT    | Installation location.                  |         |
|     ✔️     | `architecture` | TEXT    | Architecture of the package.            |         |
|           | `description`  | TEXT    | Description of the package.             |         |
|           | `size`         | BIGINT  | Size of the package in bytes.           |         |
|     ✔️     | `format`       | TEXT    | Format of the package (e.g., RPM, DEB). |         |

### Processes Table

```sql
CREATE TABLE processes (
    pid TEXT,
    name TEXT,
    ppid BIGINT,
    cmd TEXT,
    argvs TEXT,
    euser TEXT,
    ruser TEXT,
    suser TEXT,
    egroup TEXT,
    rgroup TEXT,
    sgroup TEXT,
    start_time BIGINT,
    tgid BIGINT,
    tty BIGINT,
    PRIMARY KEY (pid)
) WITHOUT ROWID;
```

This table stores information about running processes.

| Mandatory | Column       | Type   | Description                        | Default |
| :-------: | ------------ | ------ | ---------------------------------- | ------- |
|     ✔️     | `pid`        | TEXT   | Process ID.                       |         |
|           | `name`       | TEXT   | Name of the process.              |         |
|           | `ppid`       | BIGINT | Parent process ID.                |         |
|           | `cmd`        | TEXT   | Command used to start the process.|         |
|           | `argvs`      | TEXT   | Command-line arguments.           |         |
|           | `euser`      | TEXT   | Effective user of the process.    |         |
|           | `ruser`      | TEXT   | Real user of the process.         |         |
|           | `suser`      | TEXT   | Saved user of the process.        |         |
|           | `egroup`     | TEXT   | Effective group of the process.   |         |
|           | `rgroup`     | TEXT   | Real group of the process.        |         |
|           | `sgroup`     | TEXT   | Saved group of the process.       |         |
|           | `start_time` | BIGINT | Start time in epoch format.       |         |
|           | `tgid`       | BIGINT | Thread group ID.                  |         |
|           | `tty`        | BIGINT | Terminal ID.                      |         |

### Ports Table

```sql
CREATE TABLE ports (
    protocol TEXT,
    local_ip TEXT,
    local_port BIGINT,
    remote_ip TEXT,
    remote_port BIGINT,
    tx_queue BIGINT,
    rx_queue BIGINT,
    inode BIGINT,
    state TEXT,
    pid BIGINT,
    process TEXT,
    PRIMARY KEY (inode, protocol, local_ip, local_port)
) WITHOUT ROWID;
```

This table stores information about active network ports.

| Mandatory | Column       | Type   | Description                         | Default |
| :-------: | ------------ | ------ | ----------------------------------- | ------- |
|     ✔️     | `protocol`   | TEXT   | Protocol (e.g., TCP, UDP).         |         |
|     ✔️     | `local_ip`   | TEXT   | Local IP address.                  |         |
|     ✔️     | `local_port` | BIGINT | Local port number.                 |         |
|           | `remote_ip`  | TEXT   | Remote IP address.                 |         |
|           | `remote_port`| BIGINT | Remote port number.                |         |
|           | `tx_queue`   | BIGINT | Transmit queue size.               |         |
|           | `rx_queue`   | BIGINT | Receive queue size.                |         |
|     ✔️     | `inode`      | BIGINT | Inode of the socket.               |         |
|           | `state`      | TEXT   | State of the connection.           |         |
|           | `pid`        | BIGINT | Process ID associated with the port.|         |
|           | `process`    | TEXT   | Name of the associated process.    |         |

### Networks Table

```sql
CREATE TABLE networks (
    iface TEXT,
    adapter TEXT,
    iface_type TEXT,
    state TEXT,
    mtu BIGINT,
    mac TEXT,
    tx_packets INTEGER,
    rx_packets INTEGER,
    tx_bytes BIGINT,
    rx_bytes BIGINT,
    tx_errors INTEGER,
    rx_errors INTEGER,
    tx_dropped INTEGER,
    rx_dropped INTEGER,
    proto_type TEXT,
    gateway TEXT,
    dhcp TEXT,
    metric TEXT,
    address TEXT,
    netmask TEXT,
    broadcast TEXT,
    PRIMARY KEY (iface, adapter, iface_type, proto_type, address)
) WITHOUT ROWID;
```

This table stores information about network interfaces.

| Mandatory | Column       | Type    | Description                     | Default |
| :-------: | ------------ | ------- | ------------------------------- | ------- |
|     ✔️     | `iface`      | TEXT    | Interface name.                |         |
|     ✔️     | `adapter`    | TEXT    | Adapter name.                  |         |
|     ✔️     | `iface_type` | TEXT    | Interface type (e.g., Ethernet).|         |
|           | `state`      | TEXT    | State of the interface.        |         |
|           | `mtu`        | BIGINT  | Maximum transmission unit.     |         |
|           | `mac`        | TEXT    | MAC address.                   |         |
|           | `tx_packets` | INTEGER | Transmitted packets.           |         |
|           | `rx_packets` | INTEGER | Received packets.              |         |
|           | `tx_bytes`   | BIGINT  | Transmitted bytes.             |         |
|           | `rx_bytes`   | BIGINT  | Received bytes.                |         |
|           | `tx_errors`  | INTEGER | Transmission errors.           |         |
|           | `rx_errors`  | INTEGER | Reception errors.              |         |
|     ✔️     | `proto_type` | TEXT    | Protocol type.                 |         |
|           | `gateway`    | TEXT    | Gateway address.               |         |
|           | `dhcp`       | TEXT    | DHCP status.                   |         |
|     ✔️     | `address`    | TEXT    | IP address.                    |         |
|           | `netmask`    | TEXT    | Subnet mask.                   |         |
|           | `broadcast`  | TEXT    | Broadcast address.             |         |

