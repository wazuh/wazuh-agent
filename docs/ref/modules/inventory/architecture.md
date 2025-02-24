# Architecture

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

| Mandatory | Column        | Data Type | Description                  | ECS                  | ECS Data Type |
| :-------: | ------------- | ------ | ---------------------------- | -------------------- | --------- |
|     ✔️     | `os_name`     | TEXT   | Name of the operating system.| `host.os.name`      | keyword   |
|           | `hostname`    | TEXT   | Hostname of the machine.     | `host.hostname`     | keyword   |
|           | `architecture`| TEXT   | System architecture.         | `host.architecture` | keyword   |
|           | `os_version`  | TEXT   | Version of the operating system. | `host.os.version` | keyword   |
|           | `os_codename` | TEXT   | Codename of the operating system. | `host.os.full` | keyword   |
|           | `os_build`    | TEXT   | OS build identifier.         | `host.os.kernel`   | keyword   |
|           | `os_platform` | TEXT   | Platform type.               | `host.os.platform` | keyword   |
|           | `sysname`     | TEXT   | System name.                 | `host.os.type`     | keyword   |

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

| Mandatory | Column         | Data Type | Description                     | ECS                              | ECS Data Type |
| :-------: | -------------- | ------- | ------------------------------- | -------------------------------- | --------- |
|     ✔️     | `board_serial` | TEXT    | Serial number of the motherboard. | `observer.serial_number`        | keyword   |
|           | `cpu_name`     | TEXT    | Name of the CPU.                | `host.cpu.name`                 | keyword   |
|           | `cpu_cores`    | INTEGER | Number of CPU cores.            | `host.cpu.cores`                | long      |
|           | `cpu_mhz`      | INTEGER | CPU speed in MHz.               | `host.cpu.speed`                | long      |
|           | `ram_total`    | INTEGER | Total RAM in bytes.             | `host.memory.total`             | long      |
|           | `ram_free`     | INTEGER | Free RAM in bytes.              | `host.memory.free`              | long      |
|           | `ram_usage`    | INTEGER | RAM usage as a percentage.      | `host.memory.used.percentage`   | long      |

### Hotfixes Table

```sql
CREATE TABLE hotfixes (
    hotfix TEXT,
    PRIMARY KEY (hotfix)
) WITHOUT ROWID;
```

This table stores information about system hotfixes.

| Mandatory | Column   | Data Type | Description         | ECS | ECS Data Type |
| :-------: | -------- | ---- | ------------------- | ------- | ------- |
|     ✔️     | `hotfix` | TEXT | Hotfix identifier.  | `package.hotfix.name` | keyword |

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

| Mandatory | Column         | Data Type | Description                              | ECS                  | ECS Data Type |
| :-------: | -------------- | --------- | ---------------------------------------- | -------------------- | --------- |
|     ✔️     | `name`         | TEXT      | Name of the package.                    | `package.name`      | keyword   |
|     ✔️     | `version`      | TEXT      | Version of the package.                 | `package.version`   | keyword   |
|     ✔️     | `architecture` | TEXT      | Architecture of the package.            | `package.architecture` | keyword   |
|     ✔️     | `format`       | TEXT      | Format of the package.                  | `package.type`      | keyword   |
|     ✔️     | `location`     | TEXT      | Installation location.                  | `package.path`      | keyword   |
|           | `install_time`  | TEXT      | Installation timestamp.                 | `package.installed` | date      |
|           | `description`   | TEXT      | Description of the package.             | `package.description` | keyword   |
|           | `size`         | INTEGER   | Size of the package in bytes.           | `package.size`      | long      |

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

| Mandatory | Column       | Data Type | Description                        | ECS               |  ECS Data Type |
| :-------: | ------------ | ------ | ---------------------------------- | -------------------- | ------- |
|     ✔️     | `pid`        | TEXT   | Process ID.                       | `process.pid`        | long |
|           | `name`       | TEXT   | Name of the process.              | `process.name`         | keyword |
|           | `ppid`       | BIGINT | Parent process ID.                | `process.parent.pid`   | long |
|           | `cmd`        | TEXT   | Command used to start the process.| `process.command_line` | wildcard |
|           | `argvs`      | TEXT   | Command-line arguments.           | `process.args`         | keyword |
|           | `euser`      | TEXT   | Effective user of the process.    | `process.user.id`      | keyword |
|           | `ruser`      | TEXT   | Real user of the process.         | `process.real_user.id` | keyword |
|           | `suser`      | TEXT   | Saved user of the process.        | `process.saved_user.id` | keyword |
|           | `egroup`     | TEXT   | Effective group of the process.   | `process.group.id`     | keyword |
|           | `rgroup`     | TEXT   | Real group of the process.        | `process.real_group.id` | keyword |
|           | `sgroup`     | TEXT   | Saved group of the process.       | `process.saved_group.id` | keyword |
|           | `start_time` | BIGINT | Start time in epoch format.       | `process.start`          | date |
|           | `tgid`       | BIGINT | Thread group ID.                  | `process.thread.id`      | long |
|           | `tty`        | BIGINT | Terminal ID.                      | `process.tty.char_device.major` | long |

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

| Mandatory | Column       | Data Type | Description                         | ECS | ECS Data Type |
| :-------: | ------------ | ------ | ----------------------------------- | ------- | ------- |
|     ✔️     | `inode`      | BIGINT | Inode of the socket.               | `file.inode` | long |
|     ✔️     | `protocol`   | TEXT   | Protocol.                          | `network.protocol` | keyword |
|     ✔️     | `local_ip`   | TEXT   | Local IP address.                  | `source.ip` | ip |
|     ✔️     | `local_port` | BIGINT | Local port number.                 | `source.port` |long |
|           | `remote_ip`  | TEXT   | Remote IP address.                 | `destination.ip` |  ip |
|           | `remote_port`| BIGINT | Remote port number.                | `destination.port` | long |
|           | `tx_queue`   | BIGINT | Transmit queue size.               | `host.network.egress.queue` | long |
|           | `rx_queue`   | BIGINT | Receive queue size.                | `host.network.ingress.queue` | long |
|           | `state`      | TEXT   | State of the connection.           | `interface.state` | keyword |
|           | `pid`        | BIGINT | Process ID associated with the port.| `process.pid` | long |
|           | `process`    | TEXT   | Name of the associated process.    | `process.name` | keyword |

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

| Mandatory | Column       | Data Type | Description                     | ECS | ECS Data Type |
| :-------: | ------------ | ------- | ------------------------------- | ------- | ------- |
|     ✔️     | `iface`      | TEXT    | Interface name.                | `observer.ingress.interface.name` | keyword |
|     ✔️     | `adapter`    | TEXT    | Adapter name.                  | `observer.ingress.interface.alias` | keyword |
|     ✔️     | `iface_type` | TEXT    | Interface type.                | `interface.type`| keyword |
|     ✔️     | `proto_type` | TEXT    | Protocol type.                 | `network.type` | keyword |
|     ✔️     | `address`    | TEXT    | IP address.                    | `host.ip` | ip |
|           | `mac`        | TEXT    | MAC address.                   | `host.mac` | keyword |
|           | `tx_packets` | INTEGER | Transmitted packets.           | `host.network.egress.packets` | long |
|           | `rx_packets` | INTEGER | Received packets.              | `host.network.ingress.packets` | long |
|           | `tx_bytes`   | BIGINT  | Transmitted bytes.             | `host.network.egress.bytes` | long |
|           | `rx_bytes`   | BIGINT  | Received bytes.                | `host.network.ingress.bytes` | long |
|           | `tx_errors`  | INTEGER | Transmission errors.           | `host.network.egress.errors` | long |
|           | `rx_errors`  | INTEGER | Reception errors.              | `host.network.ingress.errors` | long |
|           | `tx_dropped` | INTEGER | Transmission errors.           | `host.network.egress.drops` | long |
|           | `rx_dropped` | INTEGER | Reception errors.              | `host.network.ingress.drops` | long |
|           | `mtu`        | BIGINT  | Maximum transmission unit.     | `interface.mtu` | long |
|           | `state`      | TEXT    | State of the interface.        | `interface.state` | keyword |
|           | `broadcast`  | TEXT    | Broadcast address.             | `network.broadcast` | ip |
|           | `dhcp`       | TEXT    | DHCP status.                   | `network.dhcp` | keyword |
|           | `gateway`    | TEXT    | Gateway address.               | `network.gateway` | ip |
|           | `metric`     | TEXT    | Metric of the network protocol.| `network.metric` | keyword |
|           | `netmask`    | TEXT    | Subnet mask.                   | `network.netmask` | ip |