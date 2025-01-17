# Configuration

## Logcollector Module

```yaml
logcollector:
  enabled: true
  localfiles:
    - location: /var/log/*.log
  windows:
    - channel: Application
      query: Event[System/EventID = 4624]
```

### Reference

|Mandatory|Option|Description|Default|
|:-:|--|--|--|
||`enabled`|Sets the module as enabled.|yes|