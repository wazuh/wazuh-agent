# Logcollector Module

## Introduction

**Logcollector** is the agent module that adds the ability to collect system logs,
both by reading plain text files and by collecting messages from the operating
system API.

## Configuration

| Mandatory | Option    | Description                | Default |
| :-------: | --------- | -------------------------- | ------- |
|           | `enabled` | Sets the module as enabled | yes     |

### File Collector

```yaml
logcollector:
  enabled: true
  reload_interval: 1m
  read_interval: 500ms
  localfiles:
    - /var/log/auth.log
```

The File collector handles plain-text log files. It needs a file path to work.

| Mandatory | Option          | Description                                              | Default |
| :-------: | --------------- | -------------------------------------------------------- | ------- |
|           | reload_interval | Time in milliseconds to recheck for new files to monitor | 60000   |
|           | read_interval   | Time in milliseconds to recheck for available logs       | 500     |
|     ✔️     | localfiles      | Vector of file paths to monitor                          |         |

```json
{"collector":"file","module":"logcollector"}
{"event":{"created":"2025-01-22T21:45:01.916Z","original":"2025-01-22T18:45:01.555243-03:00 box CRON[23505]: pam_unix(cron:session): session closed for user root"},"log":{"file":{"path":"/var/log/auth.log"}}}
```

### Journald Collector

```yaml
logcollector:
  enabled: true
  read_interval: 500ms
  journald:
    - field: "_SYSTEMD_UNIT"
      value: "cron.service"
      exact_match: true
      ignore_if_missing: true
    - field: "SYSLOG_IDENTIFIER"
      value: "systemd"
      exact_match: false
      ignore_if_missing: true
    - conditions:
        - field: "_COMM"
          value: "cat"
          exact_match: true
        - field: "PRIORITY"
          value: "3|4"
          exact_match: true
      ignore_if_missing: true
```

This collector gets logs from Journald on Linux. It needs a field and a value to work.

```json
{"agent":{"groups":[],"host":{"architecture":"x86_64","hostname":"HOSTNAME","ip":["LOCALIP","4444:4444:4444:4444:4444:44444:4444:4444","127.0.0.1","::1"],"os":{"name":"Ubuntu 24.01","type":"Unknown","version":"24.04"}},"id":"4444-4444-4444-4444-ae5a7d59936c","name":"","type":"Endpoint","version":"x.y.z"}}
{"module":"logcollector","collector":"journald"}
{"event":{"created":"2025-01-17T17:58:26.212Z","original":"hello0003","provider":"unknown"}}
{"module":"logcollector","collector":"journald"}
{"event":{"created":"2025-01-17T17:58:32.026Z","original":"(CRON) INFO (pidfile fd = 3)","provider":"cron.service"}}
{"module":"logcollector","collector":"journald"}
{"event":{"created":"2025-01-17T17:58:32.110Z","original":"Stopping Regular background program processing daemon...","provider":"init.scope"}}
```

| Mandatory | Option                     | Description                                                                                  | Default |
| :-------: | -------------------------- | -------------------------------------------------------------------------------------------- | ------- |
|           | read_interval              | Time in milliseconds to recheck for available logs                                           | 500     |
|     ✔️     | journald                   | Vector of journald fields to monitor                                                         |         |
|     ✔️     | journald.field             | Journald field to be monitored                                                               |         |
|     ✔️     | journald.value             | Value of the Journald field to be filtered by                                                |         |
|           | journald.exact_match       | Boolean that allows the value setting to be a substring instead of the exact filtering value | true    |
|           | journald.ignore_if_missing | Boolean to ignore the filtering condition for logs without the specified field               | false   |
|           | journald.conditions        | Vector of journald fields to filter to be applied simultaneously                             |         |

### Agent-side Syslog listener

The Wazuh agent can optionally receive basic Syslog messages directly over UDP or
TCP through the Logcollector module. This is useful for lightweight remote-site
deployments where nearby devices or applications need to forward Syslog to a local
Wazuh agent, while preserving the agent-based collection model. Received messages
enter the normal Logcollector processing path and are sent through the standard
agent-to-manager pipeline, so they remain associated with the receiving agent.

No listener is started unless it is explicitly configured. Each `syslog` entry
defines one listener (one protocol, bind address and port). Several entries may be
combined to run multiple independent listeners on the same agent.

```yaml
logcollector:
  enabled: true
  syslog:
    # UDP listener on localhost
    - protocol: udp
      bind_address: 127.0.0.1
      port: 5514
    # UDP listener on a specific interface
    - protocol: udp
      bind_address: 192.168.10.20
      port: 5515
    # TCP listener on localhost
    - protocol: tcp
      bind_address: 127.0.0.1
      port: 1514
```

`bind_address` is optional and defaults to `127.0.0.1`. Binding to `0.0.0.0` (all
interfaces) must be configured explicitly. Each message is forwarded with the
`remote-syslog` collector type and the listener identity (`<protocol>:<address>:<port>`)
as its provider:

```json
{"module":"logcollector","collector":"remote-syslog"}
{"event":{"created":"2025-01-17T17:58:26.212Z","original":"<13>Jun 25 10:00:00 testhost testapp: UDP listener test","provider":"udp:127.0.0.1:5514"}}
```

| Mandatory | Option              | Description                                                              | Default   |
| :-------: | ------------------- | ----------------------------------------------------------------------- | --------- |
|     ✔️     | syslog              | Vector of agent-side syslog listeners                                   |           |
|     ✔️     | syslog.protocol     | Listener transport protocol: `udp` or `tcp`                             |           |
|     ✔️     | syslog.port         | Listener port (1-65535)                                                  |           |
|           | syslog.bind_address | Address the listener binds to                                           | 127.0.0.1 |

A listener definition is rejected (and the listener is not started) when the
protocol is not `udp`/`tcp`, the port is missing or out of range, the bind address
is malformed, or another listener already uses the same protocol, bind address and
port combination.

> **Note:** For high-volume Syslog ingestion, TLS Syslog, disk-assisted queues,
> advanced filtering, transformations, routing, or complex parsing pipelines, use
> rsyslog, syslog-ng, Logstash, or the Wazuh manager remote Syslog input as
> appropriate. Source IP filtering for the agent-side listener should be handled
> with host firewall rules.

#### Limitations and future work

This first version implements only the UDP/TCP IP-socket listeners from
[wazuh/wazuh#15178](https://github.com/wazuh/wazuh/issues/15178). The following are
intentionally **not** included yet and are tracked as future work:

| Not yet supported | Notes |
| ----------------- | ----- |
| UNIX domain sockets (`unix_stream`, `unix_dgram`, `unix_seq`) | Local socket ingress requested in #15178; reuses the Boost.Asio local-socket pattern. |
| Named pipe / FIFO (and Windows named pipes) | Pipe ingress requested in #15178; equivalent to the legacy `syslog-pipe` format. |
| TLS Syslog (TCP) | The TCP listener is plaintext; use rsyslog/syslog-ng for TLS. |
| TCP octet-counting framing (RFC 6587) | Only newline-delimited ("non-transparent") framing is parsed. Octet-counted messages (`<len> <msg>`) are not auto-detected. |
| Hostname `bind_address` | Only numeric IP literals (IPv4/IPv6) are accepted; DNS names are rejected. |
| `allowed-ips` source filtering | Restrict senders with host firewall rules until implemented. |

### Windows Collector

```yaml
logcollector:
  enabled: true
  reload_interval: 1m
  windows:
    - channel: System
      query: Event[System/EventID = 37]
```

This collector gets logs from the Windows Event Viewer. It needs a channel and a query to work (if this is empty '*' wildcard is assumed collecting all possible messages). It will subscribe to the before mentioned channel with the specified query and it will return a json containing the events matching the criteria.

```json
{"agent":{"groups":[],"host":{"architecture":"x86_64","hostname":"HOSTNAME","ip":["LOCALIP","4444:4444:4444:4444:4444:44444:4444:4444","127.0.0.1","::1"],"os":{"name":"Microsoft Windows Server 2022","type":"Unknown","version":"10.0.20348.2762"}},"id":"4444-4444-4444-4444-ae5a7d59936c","name":"","type":"Endpoint","version":"x.y.z"}}
{"module":"logcollector","collector":"windows-eventlog"}
{"event":{"created":"2025-01-07T21:41:27.712Z","original":"<Event xmlns='http://schemas.microsoft.com/win/2004/08/events/event'><System><Provider Name='Microsoft-Windows-Time-Service' Guid='{06edcfeb-0fd0-4e53-acca-a6f8bbf81bcb}'/><EventID>37</EventID><Version>0</Version><Level>4</Level><Task>0</Task><Opcode>0</Opcode><Keywords>0x8000000000000000</Keywords><TimeCreated SystemTime='2025-01-07T21:41:26.8876581Z'/><EventRecordID>13597</EventRecordID><Correlation/><Execution ProcessID='12848' ThreadID='14956'/><Channel>System</Channel><Computer>HOSTNAME</Computer><Security UserID='S-1-5-19'/></System><EventData Name='TMP_EVENT_TIME_SOURCE_REACHABLE'><Data Name='TimeSource'>time.windows.com,0x8 (ntp.m|0x8|0.0.0.0:123-&gt;40.444.4.444:444)</Data></EventData></Event>","provider":"System"}}
```

| Mandatory | Option          | Description                                       | Default |
| :-------: | --------------- | ------------------------------------------------- | ------- |
|           | reload_interval | Time in milliseconds to recheck for subscriptions | 60000   |
|     ✔️     | windows         | Vector of readers to subscribe                    |         |
|     ✔️     | windows.channel | Channel name to be used for subscription          |         |
|           | windows.query   | Query to apply to the channel                     |         |

### macOS (ULS) Collector

```yaml
logcollector:
  enabled: true
  read_interval: 500ms
  macos:
    - query: process == "sshd" OR message CONTAINS "invalid"
      level: info
      type: trace,activity,log
```

This collector gets logs from macOS through the Unified Logging System. It needs a query, a level, and a type to work.

```json
{"agent":{"groups":[],"host":{"architecture":"x86_64","hostname":"HOSTNAME","ip":["LOCALIP","4444:4444:4444:4444:4444:44444:4444:4444","127.0.0.1","::1"],"os":{"name":"macOS","type":"Unknown","version":"15"}},"id":"4444-4444-4444-4444-ae5a7d59936c","name":"","type":"Endpoint","version":"x.y.z"}}
{"module":"logcollector","collector":"macos-uls"}
{"event":{"created":"2025-01-17T20:40:56.072Z","original":"2025-01-17 20:40:55 +0000 Setting mfgr data:<private> len:9 id:4C","provider":"macos-uls"}}
{"module":"logcollector","collector":"macos-uls"}
{"event":{"created":"2025-01-17T20:40:56.072Z","original":"2025-01-17 20:40:55 +0000 APSMessageStore - Destroying database.","provider":"macos-uls"}}
{"module":"logcollector","collector":"macos-uls"}
{"event":{"created":"2025-01-17T20:40:56.072Z","original":"2025-01-17 20:40:55 +0000 WPDScanManager - Notifying Client About Discovered Device: Client (<private>) - UUID (<private>)","provider":"macos-uls"}}
{"module":"logcollector","collector":"macos-uls"}
{"event":{"created":"2025-01-17T20:40:56.073Z","original":"2025-01-17 20:40:55 +0000 Device found changed: CBDevice 555E3FA6-AE3A-412A-6C97-2E7E2A069AD3, BDA 60:06:E3:8F:14:AF, Nm 'Bluetooth Device', DsFl 0x40 < NearbyInfo >, RSSI -44, Ch 37, AdTsMC <28588540497>, AMfD <4c 00 10 05 2c 98 bc 13 ff>, nbIAT <bc 13 ff>, nbIF 0xCA < Ranging AUE AT Duet >, CF 0x200000000 < RSSI >","provider":"macos-uls"}}
```

| Mandatory | Option        | Description                                                             | Default |
| :-------: | ------------- | ----------------------------------------------------------------------- | ------- |
|           | read_interval | Time in milliseconds to recheck for available logs                      | 500     |
|     ✔️     | macos         | Vector of queries to filter macOS logs                                  |         |
|     ✔️     | macos.query   | Predicate to filter logs                                                |         |
|     ✔️     | macos.level   | Log verbosity level: debug, info, notice, error or fault                |         |
|     ✔️     | macos.type    | Limits the log type; possible values (combinable): activity, log, trace |         |

