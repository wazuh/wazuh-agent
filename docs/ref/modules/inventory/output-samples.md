## Index:

| Type | Stateful | Stateless |
| --- | ---  | ---  |
| System | [Stateful](#System-Windows-Stateful) | [Stateful](#System-Windows-Stateless) |
| Hardware | [Stateful](#Hardware-Windows-Stateful) | [Stateless](#Hardware-Windows-Stateless) |
| Packages | [Stateful](#Packages-Windows-Stateful) | [Stateless](#Packages-Windows-Stateless) |
| Processes |[Stateful](#Processes-Windows-Stateful) | [Stateless](#Processes-Windows-Stateless) |
| Networks | [Stateful](#Networks-Windows-Stateful) | [Stateless](#Networks-Windows-Stateless) |
| Ports | [Stateful](#Ports-Windows-Stateful) | [Stateless](#Ports-Windows-Stateless) |
| Hotfix | [Stateful](#Hotfix-Windows-Stateful) | [Stateless](#Hotfix-Windows-Stateless) |

## System

<a name="System-Windows-Stateful"></a>Stateful

```json
{
    "agent": {
        "groups": [],
        "host": {
            "architecture": "x86_64",
            "hostname": "WIN-D75P8GSAED8",
            "ip": [
                "192.168.0.246",
                "fe80::3f18:94ee:b0d8:a0b9",
                "127.0.0.1",
                "::1"
            ],
            "os": {
                "name": "Microsoft Windows Server 2022 Datacenter Evaluation",
                "type": "Unknown",
                "version": "10.0.20348.3207"
            }
        },
        "id": "80a25e2d-8e52-4591-bd68-dd7e8a7baf33",
        "name": "dummy",
        "type": "Endpoint",
        "version": "v5.0.0"
    }
}
{
    "collector": "system",
    "id": "6641e605a6c9c9484124496359fff9d14c0b68b7",
    "module": "inventory",
    "operation": "create"
}
{
    "@timestamp": "2025-03-11T19:12:36.629Z",
    "host": {
        "architecture": "x86_64",
        "hostname": "WIN-D75P8GSAED8",
        "os": {
            "full": null,
            "kernel": "20348.3207",
            "name": "Microsoft Windows Server 2022 Datacenter Evaluation",
            "platform": "windows",
            "type": null,
            "version": "10.0.20348.3207"
        }
    }
}
```

<a name="System-Windows-Stateless"></a>Stateless

```json
{
    "agent": {
        "groups": [],
        "host": {
            "architecture": "x86_64",
            "hostname": "WIN-D75P8GSAED8",
            "ip": [
                "192.168.0.246",
                "fe80::3f18:94ee:b0d8:a0b9",
                "127.0.0.1",
                "::1"
            ],
            "os": {
                "name": "Microsoft Windows Server 2022 Datacenter Evaluation",
                "type": "Unknown",
                "version": "10.0.20348.3207"
            }
        },
        "id": "80a25e2d-8e52-4591-bd68-dd7e8a7baf33",
        "name": "dummy",
        "type": "Endpoint",
        "version": "v5.0.0"
    }
}
{
    "collector": "system",
    "module": "inventory"
}
{
    "event": {
        "action": "system-updated",
        "category": [
            "host"
        ],
        "type": [
            "change"
        ],
        "created": "2025-03-11T19:35:58.237Z",
        "reason": "System NewName is running OS version 10.0.20348.3207",
    },
    "system": {
      "@timestamp": "2025-03-11T20:12:46.325Z",
      "host": {
          "architecture": "x86_64",
          "hostname": "NewName",
          "os": {
              "full": null,
              "kernel": "20348.3207",
              "name": "Microsoft Windows Server 2022 Datacenter Evaluation",
              "platform": "windows",
              "type": null,
              "version": "10.0.20348.3207"
          }
      }
    }
}
```

## Hardware

<a name="Hardware-Windows-Stateful"></a>Stateful

```json
{
    "agent": {
        "groups": [],
        "host": {
            "architecture": "x86_64",
            "hostname": "WIN-D75P8GSAED8",
            "ip": [
                "192.168.0.246",
                "fe80::3f18:94ee:b0d8:a0b9",
                "127.0.0.1",
                "::1"
            ],
            "os": {
                "name": "Microsoft Windows Server 2022 Datacenter Evaluation",
                "type": "Unknown",
                "version": "10.0.20348.3207"
            }
        },
        "id": "80a25e2d-8e52-4591-bd68-dd7e8a7baf33",
        "name": "dummy",
        "type": "Endpoint",
        "version": "v5.0.0"
    }
}
{
    "collector": "hardware",
    "id": "cfd8af98962a0566de4bb2dcc0773dfc6cca0ac4",
    "module": "inventory",
    "operation": "create"
}
{
    "@timestamp": "2025-03-11T19:34:57.857Z",
    "host": {
        "cpu": {
            "cores": 16,
            "name": "QEMU Virtual CPU version 2.5+",
            "speed": 3793
        },
        "memory": {
            "free": 14086392,
            "total": 16512084,
            "used": {
                "percentage": 14
            }
        }
    },
    "observer": {
        "serial_number": null
    }
}



```

<a name="Hardware-Windows-Stateless"></a>Stateless

```json
{
    "agent": {
        "groups": [],
        "host": {
            "architecture": "x86_64",
            "hostname": "WIN-D75P8GSAED8",
            "ip": [
                "192.168.0.246",
                "fe80::3f18:94ee:b0d8:a0b9",
                "127.0.0.1",
                "::1"
            ],
            "os": {
                "name": "Microsoft Windows Server 2022 Datacenter Evaluation",
                "type": "Unknown",
                "version": "10.0.20348.3207"
            }
        },
        "id": "80a25e2d-8e52-4591-bd68-dd7e8a7baf33",
        "name": "dummy",
        "type": "Endpoint",
        "version": "v5.0.0"
    }
}
{
    "collector": "hardware",
    "module": "inventory"
}
{
    "event": {
        "action": "hardware-updated",
        "category": [
            "host"
        ],
        "changed_fields": [
            "host.memory.free",
            "host.memory.used.percentage"
        ],
        "created": "2025-03-11T19:35:58.237Z",
        "reason": "Hardware changed",
        "type": [
            "change"
        ]
    },
    "host": {
        "cpu": {
            "cores": 16,
            "name": "QEMU Virtual CPU version 2.5+",
            "speed": 3793
        },
        "memory": {
            "free": 13292072,
            "previous": {
                "free": 13950732
            },
            "total": 16512084,
            "used": {
                "percentage": 19,
                "previous": {
                    "percentage": 15
                }
            }
        }
    },
    "observer": {
        "serial_number": null
    }
}
```

## Packages

<a name="Packages-Windows-Stateful"></a>Stateful

```json
{
    "agent": {
        "groups": [],
        "host": {
            "architecture": "x86_64",
            "hostname": "WIN-D75P8GSAED8",
            "ip": [
                "192.168.0.246",
                "fe80::3f18:94ee:b0d8:a0b9",
                "127.0.0.1",
                "::1"
            ],
            "os": {
                "name": "Microsoft Windows Server 2022 Datacenter Evaluation",
                "type": "Unknown",
                "version": "10.0.20348.3207"
            }
        },
        "id": "80a25e2d-8e52-4591-bd68-dd7e8a7baf33",
        "name": "dummy",
        "type": "Endpoint",
        "version": "v5.0.0"
    }
}
{
    "collector": "packages",
    "id": "2342693b1df78e2dc880137449fa9b8ff7c0db40",
    "module": "inventory",
    "operation": "create"
}
{
    "@timestamp": "2025-03-11T19:34:57.857Z",
    "package": {
        "architecture": "x86_64",
        "description": null,
        "installed": "2024-10-05T00:38:12.000Z",
        "name": "Git",
        "path": "C:\\Program Files\\Git\\",
        "size": null,
        "type": "win",
        "version": "2.46.2"
    }
}
{
    "collector": "packages",
    "id": "4dfe0341534b85d67559548ee948caa4328fcae7",
    "module": "inventory",
    "operation": "create"
}
{
    "@timestamp": "2025-03-11T19:34:57.857Z",
    "package": {
        "architecture": "x86_64",
        "description": null,
        "installed": "2025-03-11T22:17:19.000Z",
        "name": "Mozilla Firefox (x64 en-US)",
        "path": "C:\\Program Files\\Mozilla Firefox",
        "size": null,
        "type": "win",
        "version": "136.0"
    }
}
{
    "collector": "packages",
    "id": "f3615bddf91aa92718e046bbf97912fbb4ddccb0",
    "module": "inventory",
    "operation": "create"
}
{
    "@timestamp": "2025-03-11T19:34:57.857Z",
    "package": {
        "architecture": "x86_64",
        "description": null,
        "installed": "2024-10-05T03:01:12.000Z",
        "name": "Mozilla Maintenance Service",
        "path": null,
        "size": null,
        "type": "win",
        "version": "131.0"
    }
}
```

<a name="Packages-Windows-Stateless"></a>Stateless

```json
{
    "agent": {
        "groups": [],
        "host": {
            "architecture": "x86_64",
            "hostname": "WIN-D75P8GSAED8",
            "ip": [
                "192.168.0.246",
                "fe80::3f18:94ee:b0d8:a0b9",
                "127.0.0.1",
                "::1"
            ],
            "os": {
                "name": "Microsoft Windows Server 2022 Datacenter Evaluation",
                "type": "Unknown",
                "version": "10.0.20348.3207"
            }
        },
        "id": "80a25e2d-8e52-4591-bd68-dd7e8a7baf33",
        "name": "dummy",
        "type": "Endpoint",
        "version": "v5.0.0"
    }
}
{
    "collector": "packages",
    "module": "inventory"
}
{
    "event": {
        "action": "package-updated",
        "category": [
            "package"
        ],
        "changed_fields": [
            "package.size"
        ],
        "created": "2025-03-11T20:28:04.428Z",
        "reason": "Package Git updated",
        "type": [
            "change"
        ]
    },
    "package": {
        "architecture": "x86_64",
        "description": null,
        "installed": "2024-10-05T00:38:12.000Z",
        "name": "Git",
        "path": "C:\\Program Files\\Git\\",
        "previous": {
            "size": 0
        },
        "size": null,
        "type": "win",
        "version": "2.46.2"
    }
}

```

## Processes

<a name="Processes-Windows-Stateful"></a>Stateful

```json
{
    "agent": {
        "groups": [],
        "host": {
            "architecture": "x86_64",
            "hostname": "WIN-D75P8GSAED8",
            "ip": [
                "192.168.0.246",
                "fe80::3f18:94ee:b0d8:a0b9",
                "127.0.0.1",
                "::1"
            ],
            "os": {
                "name": "Microsoft Windows Server 2022 Datacenter Evaluation",
                "type": "Unknown",
                "version": "10.0.20348.3207"
            }
        },
        "id": "80a25e2d-8e52-4591-bd68-dd7e8a7baf33",
        "name": "dummy",
        "type": "Endpoint",
        "version": "v5.0.0"
    }
}
{
    "collector": "processes",
    "id": "bd4ee971a14c65a1db0e1030f8e55896515bbc25",
    "module": "inventory",
    "operation": "create"
}
{
    "@timestamp": "2025-03-11T19:34:57.857Z",
    "process": {
        "args": null,
        "command_line": "C:\\Windows\\System32\\svchost.exe",
        "group": {
            "id": null
        },
        "name": "svchost.exe",
        "parent": {
            "pid": 952
        },
        "pid": "5196",
        "real_group": {
            "id": null
        },
        "real_user": {
            "id": null
        },
        "saved_group": {
            "id": null
        },
        "saved_user": {
            "id": null
        },
        "start": 1741702624,
        "thread": {
            "id": null
        },
        "tty": {
            "char_device": {
                "major": null
            }
        },
        "user": {
            "id": null
        }
    }
}
{
    "collector": "processes",
    "id": "ad1493249b1f766ce4829f4c3361b2f45184d688",
    "module": "inventory",
    "operation": "create"
}
{
    "@timestamp": "2025-03-11T19:34:57.857Z",
    "process": {
        "args": null,
        "command_line": "C:\\Windows\\System32\\sihost.exe",
        "group": {
            "id": null
        },
        "name": "sihost.exe",
        "parent": {
            "pid": 2628
        },
        "pid": "5628",
        "real_group": {
            "id": null
        },
        "real_user": {
            "id": null
        },
        "saved_group": {
            "id": null
        },
        "saved_user": {
            "id": null
        },
        "start": 1741702624,
        "thread": {
            "id": null
        },
        "tty": {
            "char_device": {
                "major": null
            }
        },
        "user": {
            "id": null
        }
    }
}
{
    "collector": "processes",
    "id": "7cc34b5d048ca9fbbe6a4be3ad1c7152515dd6f9",
    "module": "inventory",
    "operation": "create"
}
{
    "@timestamp": "2025-03-11T19:34:57.857Z",
    "process": {
        "args": null,
        "command_line": "C:\\Windows\\System32\\svchost.exe",
        "group": {
            "id": null
        },
        "name": "svchost.exe",
        "parent": {
            "pid": 952
        },
        "pid": "1524",
        "real_group": {
            "id": null
        },
        "real_user": {
            "id": null
        },
        "saved_group": {
            "id": null
        },
        "saved_user": {
            "id": null
        },
        "start": 1741702624,
        "thread": {
            "id": null
        },
        "tty": {
            "char_device": {
                "major": null
            }
        },
        "user": {
            "id": null
        }
    }
}

```

<a name="Processes-Windows-Stateless"></a>Stateless

```json
{
    "agent": {
        "groups": [],
        "host": {
            "architecture": "x86_64",
            "hostname": "WIN-D75P8GSAED8",
            "ip": [
                "192.168.0.246",
                "fe80::3f18:94ee:b0d8:a0b9",
                "127.0.0.1",
                "::1"
            ],
            "os": {
                "name": "Microsoft Windows Server 2022 Datacenter Evaluation",
                "type": "Unknown",
                "version": "10.0.20348.3207"
            }
        },
        "id": "80a25e2d-8e52-4591-bd68-dd7e8a7baf33",
        "name": "dummy",
        "type": "Endpoint",
        "version": "v5.0.0"
    }
}
{
    "collector": "processes",
    "module": "inventory"
}
{
    "event": {
        "action": "process-started",
        "category": [
            "process"
        ],
        "created": "2025-03-11T19:35:58.237Z",
        "reason": "Process wuauclt.exe (PID: wuauclt.exe) was started",
        "type": [
            "start"
        ]
    },
    "process": {
        "args": null,
        "command_line": "C:\\Windows\\System32\\wuauclt.exe",
        "group": {
            "id": null
        },
        "name": "wuauclt.exe",
        "parent": {
            "pid": 1444
        },
        "pid": "2540",
        "real_group": {
            "id": null
        },
        "real_user": {
            "id": null
        },
        "saved_group": {
            "id": null
        },
        "saved_user": {
            "id": null
        },
        "start": 1741721749,
        "thread": {
            "id": null
        },
        "tty": {
            "char_device": {
                "major": null
            }
        },
        "user": {
            "id": null
        }
    }
}
{
    "collector": "processes",
    "module": "inventory"
}
{
    "event": {
        "action": "process-started",
        "category": [
            "process"
        ],
        "created": "2025-03-11T19:35:58.237Z",
        "reason": "Process AM_Delta_Patch_1.423.343.0.exe (PID: AM_Delta_Patch_1.423.343.0.exe) was started",
        "type": [
            "start"
        ]
    },
    "process": {
        "args": null,
        "command_line": "C:\\Windows\\SoftwareDistribution\\Download\\Install\\AM_Delta_Patch_1.423.343.0.exe",
        "group": {
            "id": null
        },
        "name": "AM_Delta_Patch_1.423.343.0.exe",
        "parent": {
            "pid": 2540
        },
        "pid": "5768",
        "real_group": {
            "id": null
        },
        "real_user": {
            "id": null
        },
        "saved_group": {
            "id": null
        },
        "saved_user": {
            "id": null
        },
        "start": 1741721751,
        "thread": {
            "id": null
        },
        "tty": {
            "char_device": {
                "major": null
            }
        },
        "user": {
            "id": null
        }
    }
}

```

## Networks


<a name="Networks-Windows-Stateful"></a>Stateful

```json
{
    "agent": {
        "groups": [],
        "host": {
            "architecture": "x86_64",
            "hostname": "WIN-D75P8GSAED8",
            "ip": [
                "192.168.0.246",
                "fe80::3f18:94ee:b0d8:a0b9",
                "127.0.0.1",
                "::1"
            ],
            "os": {
                "name": "Microsoft Windows Server 2022 Datacenter Evaluation",
                "type": "Unknown",
                "version": "10.0.20348.3207"
            }
        },
        "id": "80a25e2d-8e52-4591-bd68-dd7e8a7baf33",
        "name": "dummy",
        "type": "Endpoint",
        "version": "v5.0.0"
    }
}
{
    "collector": "networks",
    "module": "inventory"
}
{
    "event": {
        "action": "network-interface-updated",
        "category": [
            "network"
        ],
        "changed_fields": [
            "host.network.ingress.bytes",
            "host.network.ingress.packets",
            "host.network.egress.bytes",
            "host.network.egress.packets"
        ],
        "created": "2025-03-11T19:35:28.175Z",
        "reason": "Network interface Ethernet updated",
        "type": [
            "change"
        ]
    },
    "host": {
        "ip": [
            "192.168.0.246"
        ],
        "mac": "bc:24:11:e1:ea:20",
        "network": {
            "egress": {
                "bytes": 25423945,
                "drops": 0,
                "errors": 0,
                "packets": 26291,
                "previous": {
                    "bytes": 25254491,
                    "packets": 26055
                }
            },
            "ingress": {
                "bytes": 21117598,
                "drops": 383,
                "errors": 0,
                "packets": 31245,
                "previous": {
                    "bytes": 20689575,
                    "packets": 30849
                }
            }
        }
    },
    "interface": {
        "mtu": 1500,
        "state": "up",
        "type": "ethernet"
    },
    "network": {
        "broadcast": [
            "192.168.0.255"
        ],
        "dhcp": "enabled",
        "gateway": [
            "192.168.0.1"
        ],
        "metric": "15",
        "netmask": [
            "255.255.255.0"
        ],
        "type": "ipv4"
    },
    "observer": {
        "ingress": {
            "interface": {
                "alias": "Red Hat VirtIO Ethernet Adapter",
                "name": "Ethernet"
            }
        }
    }
}

```

<a name="Networks-Windows-Stateless"></a>Stateless

```json
{
    "agent": {
        "groups": [],
        "host": {
            "architecture": "x86_64",
            "hostname": "WIN-D75P8GSAED8",
            "ip": [
                "192.168.0.246",
                "fe80::3f18:94ee:b0d8:a0b9",
                "127.0.0.1",
                "::1"
            ],
            "os": {
                "name": "Microsoft Windows Server 2022 Datacenter Evaluation",
                "type": "Unknown",
                "version": "10.0.20348.3207"
            }
        },
        "id": "80a25e2d-8e52-4591-bd68-dd7e8a7baf33",
        "name": "dummy",
        "type": "Endpoint",
        "version": "v5.0.0"
    }
}
{
    "collector": "networks",
    "module": "inventory"
}
{
    "event": {
        "action": "network-interface-updated",
        "category": [
            "network"
        ],
        "changed_fields": [
            "host.network.ingress.bytes",
            "host.network.ingress.drops",
            "host.network.ingress.packets",
            "host.network.egress.bytes",
            "host.network.egress.packets"
        ],
        "created": "2025-03-11T19:35:58.237Z",
        "reason": "Network interface Ethernet updated",
        "type": [
            "change"
        ]
    },
    "host": {
        "ip": [
            "fe80::3f18:94ee:b0d8:a0b9"
        ],
        "mac": "bc:24:11:e1:ea:20",
        "network": {
            "egress": {
                "bytes": 25893293,
                "drops": 0,
                "errors": 0,
                "packets": 31973,
                "previous": {
                    "bytes": 25423945,
                    "packets": 26291
                }
            },
            "ingress": {
                "bytes": 73220097,
                "drops": 384,
                "errors": 0,
                "packets": 66899,
                "previous": {
                    "bytes": 21117598,
                    "drops": 383,
                    "packets": 31245
                }
            }
        }
    },
    "interface": {
        "mtu": 1500,
        "state": "up",
        "type": "ethernet"
    },
    "network": {
        "broadcast": [],
        "dhcp": "enabled",
        "gateway": [
            "192.168.0.1"
        ],
        "metric": "15",
        "netmask": [
            "ffff:ffff:ffff:ffff::"
        ],
        "type": "ipv6"
    },
    "observer": {
        "ingress": {
            "interface": {
                "alias": "Red Hat VirtIO Ethernet Adapter",
                "name": "Ethernet"
            }
        }
    }
}


```

## Ports

<a name="Ports-Windows-Stateful"></a>Stateful

```json
{
    "agent": {
        "groups": [],
        "host": {
            "architecture": "x86_64",
            "hostname": "WIN-D75P8GSAED8",
            "ip": [
                "192.168.0.246",
                "fe80::3f18:94ee:b0d8:a0b9",
                "127.0.0.1",
                "::1"
            ],
            "os": {
                "name": "Microsoft Windows Server 2022 Datacenter Evaluation",
                "type": "Unknown",
                "version": "10.0.20348.3207"
            }
        },
        "id": "80a25e2d-8e52-4591-bd68-dd7e8a7baf33",
        "name": "dummy",
        "type": "Endpoint",
        "version": "v5.0.0"
    }
}
{
    "collector": "ports",
    "id": "de9ededfdfbd8a9d76dc9edf700a9e6f9f60a109",
    "module": "inventory",
    "operation": "create"
}
{
    "@timestamp": "2025-03-11T19:34:57.857Z",
    "destination": {
        "ip": [
            "0.0.0.0"
        ],
        "port": 0
    },
    "file": {
        "inode": 0
    },
    "host": {
        "network": {
            "egress": {
                "queue": null
            },
            "ingress": {
                "queue": null
            }
        }
    },
    "interface": {
        "state": "listening"
    },
    "network": {
        "protocol": "tcp"
    },
    "process": {
        "name": "sshd.exe",
        "pid": 2080
    },
    "source": {
        "ip": [
            "0.0.0.0"
        ],
        "port": 22
    }
}
{
    "collector": "ports",
    "id": "18035dfb60f7098401f704557473d06bc76cb70f",
    "module": "inventory",
    "operation": "create"
}
{
    "@timestamp": "2025-03-11T19:34:57.857Z",
    "destination": {
        "ip": [
            "0.0.0.0"
        ],
        "port": 0
    },
    "file": {
        "inode": 0
    },
    "host": {
        "network": {
            "egress": {
                "queue": null
            },
            "ingress": {
                "queue": null
            }
        }
    },
    "interface": {
        "state": "listening"
    },
    "network": {
        "protocol": "tcp"
    },
    "process": {
        "name": "svchost.exe",
        "pid": 1080
    },
    "source": {
        "ip": [
            "0.0.0.0"
        ],
        "port": 135
    }
}
```

<a name="Ports-Windows-Stateless"></a>Stateless

```json
{
    "agent": {
        "groups": [],
        "host": {
            "architecture": "x86_64",
            "hostname": "WIN-D75P8GSAED8",
            "ip": [
                "192.168.0.246",
                "fe80::3f18:94ee:b0d8:a0b9",
                "127.0.0.1",
                "::1"
            ],
            "os": {
                "name": "Microsoft Windows Server 2022 Datacenter Evaluation",
                "type": "Unknown",
                "version": "10.0.20348.3207"
            }
        },
        "id": "80a25e2d-8e52-4591-bd68-dd7e8a7baf33",
        "name": "dummy",
        "type": "Endpoint",
        "version": "v5.0.0"
    }
}
{
    "collector": "ports",
    "module": "inventory"
}
{
    "destination": {
        "ip": [
            "40.69.42.241"
        ],
        "port": 443
    },
    "event": {
        "action": "port-updated",
        "category": [
            "network"
        ],
        "changed_fields": [
            "process.name",
            "process.pid",
            "interface.state"
        ],
        "created": "2025-03-11T19:35:58.237Z",
        "reason": "Updated connection from source port 52674 to destination port 443",
        "type": [
            "change"
        ]
    },
    "file": {
        "inode": 0
    },
    "host": {
        "network": {
            "egress": {
                "queue": null
            },
            "ingress": {
                "queue": null
            }
        }
    },
    "interface": {
        "previous": {
            "state": "established"
        },
        "state": "time_wait"
    },
    "network": {
        "protocol": "tcp"
    },
    "process": {
        "name": "System Idle Process",
        "pid": 0,
        "previous": {
            "name": "svchost.exe",
            "pid": 8636
        }
    },
    "source": {
        "ip": [
            "192.168.0.246"
        ],
        "port": 52674
    }
}

```


## Hotfixes

<a name="Hotfix-Windows-Stateful"></a>Stateful

```json
{
    "agent": {
        "groups": [],
        "host": {
            "architecture": "x86_64",
            "hostname": "WIN-D75P8GSAED8",
            "ip": [
                "192.168.0.246",
                "fe80::3f18:94ee:b0d8:a0b9",
                "127.0.0.1",
                "::1"
            ],
            "os": {
                "name": "Microsoft Windows Server 2022 Datacenter Evaluation",
                "type": "Unknown",
                "version": "10.0.20348.2762"
            }
        },
        "id": "2fcc3b59-9873-44dd-832a-1e35ab02ea99",
        "name": "dummy",
        "type": "Endpoint",
        "version": "v5.0.0"
    }
}
{
    "collector": "hotfixes",
    "module": "inventory"
}
{
    "event": {
        "action": "hotfix-installed",
        "category": [
            "hotfix"
        ],
        "created": "2025-03-11T13:47:02.669Z",
        "reason": "Hotfix KB5049617 was installed",
        "type": [
            "installation"
        ]
    },
    "package": {
        "hotfix": {
            "name": "KB5049617"
        }
    }
}
```

<a name="Hotfix-Windows-Stateless"></a>Stateless

```json
{
    "agent": {
        "groups": [],
        "host": {
            "architecture": "x86_64",
            "hostname": "WIN-D75P8GSAED8",
            "ip": [
                "192.168.0.246",
                "fe80::3f18:94ee:b0d8:a0b9",
                "127.0.0.1",
                "::1"
            ],
            "os": {
                "name": "Microsoft Windows Server 2022 Datacenter Evaluation",
                "type": "Unknown",
                "version": "10.0.20348.2762"
            }
        },
        "id": "2fcc3b59-9873-44dd-832a-1e35ab02ea99",
        "name": "dummy",
        "type": "Endpoint",
        "version": "v5.0.0"
    }
}
{
    "collector": "hotfixes",
    "id": "012b05f27f5fb61830f2c2217e73b3d7ac2356e5",
    "module": "inventory",
    "operation": "create"
}
{
    "@timestamp": "2025-03-11T13:50:35.235Z",
    "package": {
        "hotfix": {
            "name": "KB5051979"
        }
    }
}
```
