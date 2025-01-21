# Configuration

## Logcollector Module

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

### Reference

| Mandatory | Option    | Description                | Default |
| :-------: | --------- | -------------------------- | ------- |
|           | `enabled` | Sets the module as enabled | yes     |
