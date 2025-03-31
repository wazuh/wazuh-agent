# Security Configuration Assessment (SCA) Module

## Introduction

The **SCA** module (Security Configuration Assessment) is responsible for assessing the security posture of the system by evaluating it against predefined policies. These policies are written in YAML and contain rules that check system configuration, permissions, and the presence or absence of specific files, commands, or settings.

The module can run automatically at agent startup and at regular intervals, scanning the system against one or more configured policies. Policies can be enabled or disabled selectively.

## Configuration

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
|           | `interval`          | Time between scans (supports `s`, `m`, `h`, `d`)                             | 1h      |
|           | `policies`          | List of enabled policy file paths                                           | —       |
|           | `policies_disabled` | List of policy file paths to explicitly disable                             | —       |

## Policy Files

Each policy file is a YAML file defining checks (rules) grouped by sections and metadata. These rules are executed during the scan, and the results are stored for reporting and compliance auditing.
