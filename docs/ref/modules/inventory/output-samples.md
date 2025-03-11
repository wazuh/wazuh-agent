## Index:

| Type | Linux | Windows | MacOS |
| --- | --- | --- | --- |
| System | [Stateful](#System-Linux-Stateful) [Stateless](#System-Linux-Stateless) |  [Stateful](#System-Windows-Stateful) [Stateless](#System-Windows-Stateless)  |  [Stateful](#System-Macos-Stateful) [Stateless](#System-Macos-Stateless) |
| Hardware | [Stateful](#Hardware-Linux-Stateful) [Stateless](#Hardware-Linux-Stateless) |  [Stateful](#Hardware-Windows-Stateful) [Stateless](#Hardware-Windows-Stateless)  |  [Stateful](#Hardware-Macos-Stateful) [Stateless](#Hardware-Macos-Stateless) |
| Packages | [Stateful](#Packages-Linux-Stateful) [Stateless](#Packages-Linux-Stateless) |  [Stateful](#Packages-Windows-Stateful) [Stateless](#Packages-Windows-Stateless)  |  [Stateful](#Packages-Macos-Stateful) [Stateless](#Packages-Macos-Stateless) |
| Processes | [Stateful](#Processes-Linux-Stateful) [Stateless](#Processes-Linux-Stateless) |  [Stateful](#Processes-Windows-Stateful) [Stateless](#Processes-Windows-Stateless)  |  [Stateful](#Processes-Macos-Stateful) [Stateless](#Processes-Macos-Stateless) |
| Networks | [Stateful](#Networks-Linux-Stateful) [Stateless](#Networks-Linux-Stateless) |  [Stateful](#Networks-Windows-Stateful) [Stateless](#Networks-Windows-Stateless)  |  [Stateful](#Networks-Macos-Stateful) [Stateless](#Networks-Macos-Stateless) |
| Ports | [Stateful](#Ports-Linux-Stateful) [Stateless](#Ports-Linux-Stateless) |  [Stateful](#Ports-Windows-Stateful) [Stateless](#Ports-Windows-Stateless)  |  [Stateful](#Ports-Macos-StatefulMacos-Stateful) [Stateless](#Ports-Macos-Stateless) |
| Hotfix |  | [Stateful](#Hotfix-Windows-Stateful) [Stateless](#Hotfix-Windows-Stateless)  |  |

## System

### Linux (Ubuntu 22.04)

<a name="System-Linux-Stateful"></a>Stateful

```json
{
  "_index": "wazuh-states-inventory-system",
  "_id": "a969a5a4f28d26f09e20b134710338ba82ba26dc",
  "_version": 1,
  "_score": 0,
  "_source": {
    "agent": {
      "id": "87676c24-26ea-4505-8618-e049b901f389",
      "name": "vm-ubuntu2204-agent",
      "groups": [],
      "type": "Endpoint",
      "version": "5.0.0",
      "host": {
        "architecture": "x86_64",
        "hostname": "vm-ubuntu2204-agent",
        "ip": [
          "10.0.2.15",
          "fe80::a00:27ff:fefc:450",
          "192.168.56.4",
          "fe80::a00:27ff:fe04:2d5b"
        ],
        "os": {
          "name": "Ubuntu",
          "type": "Linux",
          "version": "22.04.2 LTS (Jammy Jellyfish)"
        }
      }
    },
    "@timestamp": "2025-02-13T18:13:33.646Z",
    "host": {
      "architecture": "x86_64",
      "hostname": "vm-ubuntu2204-agent",
      "os": {
        "full": "jammy",
        "kernel": null,
        "name": "Ubuntu",
        "platform": "ubuntu",
        "type": "Linux",
        "version": "22.04.2 LTS (Jammy Jellyfish)"
      }
    }
  },
  "fields": {
    "@timestamp": [
      "2025-02-13T18:13:33.646Z"
    ]
  }
}
```

<a name="System-Linux-Stateless"></a>Stateless

```json
{
  "_index": "wazuh-alerts-5.x-0001",
  "_id": "TT_TAJUBUF68aH7Eocvx",
  "_version": 1,
  "_score": 0,
  "_source": {
    "@timestamp": "2025-02-13T19:40:23Z",
    "agent": {
      "groups": [],
      "host": {
        "architecture": "x86_64",
        "hostname": "vm-ubuntu2204-agent",
        "ip": [
          "10.0.2.15",
          "fe80::a00:27ff:fefc:450",
          "192.168.56.4",
          "fe80::a00:27ff:fe04:2d5b"
        ],
        "os": {
          "name": "Ubuntu",
          "type": "Linux",
          "version": "22.04.2 LTS (Jammy Jellyfish)"
        }
      },
      "id": "87676c24-26ea-4505-8618-e049b901f389",
      "name": "vm-ubuntu2204-agent",
      "type": "Endpoint",
      "version": "5.0.0"
    },
    "event": {
      "action": "system-updated",
      "category": [
        "host"
      ],
      "changed_fields": [
        "host.hostname"
      ],
      "collector": "system",
      "created": "2025-02-13T19:40:18.784Z",
      "module": "inventory",
      "reason": "System new-hostname is running OS version 22.04.2 LTS (Jammy Jellyfish)",
      "type": [
        "change"
      ]
    },
    "host": {
      "architecture": "x86_64",
      "hostname": "new-hostname",
      "os": {
        "full": "jammy",
        "kernel": null,
        "name": "Ubuntu",
        "platform": "ubuntu",
        "type": "Linux",
        "version": "22.04.2 LTS (Jammy Jellyfish)"
      },
      "previous": {
        "hostname": "vm-ubuntu2204-agent"
      }
    }
  },
  "fields": {
    "event.created": [
      "2025-02-13T19:40:18.784Z"
    ],
    "@timestamp": [
      "2025-02-13T19:40:23.000Z"
    ]
  },
  "highlight": {
    "event.action": [
      "@opensearch-dashboards-highlighted-field@system-updated@/opensearch-dashboards-highlighted-field@"
    ]
  },
  "sort": [
    0,
    null
  ]
}
```


### Windows (Windows 10)

<a name="System-Windows-Stateful"></a>Stateful

```json
{
  "_index": "wazuh-states-inventory-system",
  "_id": "a5f53551485456d2ba0cf3a4d2fab8cae9803480",
  "_version": 1,
  "_score": 0,
  "_source": {
    "agent": {
      "id": "51728df7-57d2-47b6-a065-a22995087bd7",
      "name": "DESKTOP-U8OHD3A",
      "groups": [],
      "type": "Endpoint",
      "version": "5.0.0",
      "host": {
        "architecture": "x86_64",
        "hostname": "DESKTOP-U8OHD3A",
        "ip": [
          "192.168.124.1",
          "fe80::4a97:2e71:39f4:21e5"
        ],
        "os": {
          "name": "Microsoft Windows 10 Home",
          "type": "Unknown",
          "version": "10.0.19045.5371"
        }
      }
    },
    "@timestamp": "2025-02-14T23:09:42.160Z",
    "host": {
      "architecture": "x86_64",
      "hostname": "DESKTOP-U8OHD3A",
      "os": {
        "full": null,
        "kernel": "19045.5371",
        "name": "Microsoft Windows 10 Home",
        "platform": "windows",
        "type": null,
        "version": "10.0.19045.5371"
      }
    }
  },
  "fields": {
    "@timestamp": [
      "2025-02-14T23:09:42.160Z"
    ]
  }
}

```

<a name="System-Windows-Stateless"></a>Stateless

```json

{
  "_index": "wazuh-alerts-5.x-0001",
  "_id": "NaUXFpUBJYGzKyMzWHnL",
  "_version": 1,
  "_score": null,
  "_source": {
    "@timestamp": "2025-02-17T22:46:20Z",
    "agent": {
      "groups": [],
      "host": {
        "architecture": "x86_64",
        "hostname": "DESKTOP-CHB",
        "ip": [
          "192.168.124.1",
          "fe80::4a97:2e71:39f4:21e5"
        ],
        "os": {
          "name": "Microsoft Windows 10 Home",
          "type": "Unknown",
          "version": "10.0.19045.5487"
        }
      },
      "id": "4252f628-46c1-43c4-a184-f19887e768fc",
      "name": "DESKTOP-U8OHD3A",
      "type": "Endpoint",
      "version": "5.0.0"
    },
    "event": {
      "action": "system-updated",
      "category": [
        "host"
      ],
      "changed_fields": [
        "host.hostname"
      ],
      "collector": "system",
      "created": "2025-02-17T22:45:37.335Z",
      "module": "inventory",
      "reason": "System DESKTOP-CHB is running OS version 10.0.19045.5487",
      "type": [
        "change"
      ]
    },
    "host": {
      "architecture": "x86_64",
      "hostname": "DESKTOP-CHB",
      "os": {
        "full": null,
        "kernel": "19045.5487",
        "name": "Microsoft Windows 10 Home",
        "platform": "windows",
        "type": null,
        "version": "10.0.19045.5487"
      },
      "previous": {
        "hostname": "DESKTOP-U8OHD3A"
      }
    }
  },
  "fields": {
    "event.created": [
      "2025-02-17T22:45:37.335Z"
    ],
    "@timestamp": [
      "2025-02-17T22:46:20.000Z"
    ]
  },
  "highlight": {
    "host.hostname": [
      "@opensearch-dashboards-highlighted-field@DESKTOP-CHB@/opensearch-dashboards-highlighted-field@"
    ]
  },
  "sort": [
    1739832380000
  ]
}

```

### Macos (macOS Sonoma 14.4.1)

<a name="System-Macos-Stateful"></a>Stateful

```json
{
  "_index": "wazuh-states-inventory-system",
  "_id": "162c9416c74f41e982206254cd396aa57bc4a0e9",
  "_version": 1,
  "_score": 0,
  "_source": {
    "agent": {
      "id": "e7aadfe0-1971-4351-9767-34a330be3c4e",
      "name": "idr-2097-sonoma-14-9903",
      "groups": [],
      "type": "Endpoint",
      "version": "5.0.0",
      "host": {
        "architecture": "x86_64",
        "hostname": "idr-2097-sonoma-14-9903",
        "ip": [
          "10.211.55.103",
          "fe80::5c:92d1:bd06:96e5"
        ],
        "os": {
          "name": "macOS",
          "type": "Darwin",
          "version": "14.4.1"
        }
      }
    },
    "@timestamp": "2025-02-14T15:38:37.835Z",
    "host": {
      "architecture": "x86_64",
      "hostname": "idr-2097-sonoma-14-9903",
      "os": {
        "full": "Sonoma",
        "kernel": "23E224",
        "name": "macOS",
        "platform": "darwin",
        "type": "Darwin",
        "version": "14.4.1"
      }
    }
  },
  "fields": {
    "@timestamp": [
      "2025-02-14T15:38:37.835Z"
    ]
  }
}
```

<a name="System-Macos-Stateless"></a>Stateless

```json
{
  "_index": "wazuh-alerts-5.x-0001",
  "_id": "oFA9BZUBA1q-qsf1TJoL",
  "_version": 1,
  "_score": null,
  "_source": {
    "@timestamp": "2025-02-14T16:14:17Z",
    "agent": {
      "groups": [],
      "host": {
        "architecture": "x86_64",
        "hostname": "idr-2097-sonoma-14-9903",
        "ip": [
          "10.211.55.103",
          "fe80::5c:92d1:bd06:96e5"
        ],
        "os": {
          "name": "macOS",
          "type": "Darwin",
          "version": "14.4.1"
        }
      },
      "id": "e7aadfe0-1971-4351-9767-34a330be3c4e",
      "name": "idr-2097-sonoma-14-9903",
      "type": "Endpoint",
      "version": "5.0.0"
    },
    "event": {
      "action": "system-updated",
      "category": [
        "host"
      ],
      "changed_fields": [
        "host.hostname"
      ],
      "collector": "system",
      "created": "2025-02-14T16:14:13.912Z",
      "module": "inventory",
      "reason": "System new-hostname is running OS version 14.4.1",
      "type": [
        "change"
      ]
    },
    "host": {
      "architecture": "x86_64",
      "hostname": "new-hostname",
      "os": {
        "full": "Sonoma",
        "kernel": "23E224",
        "name": "macOS",
        "platform": "darwin",
        "type": "Darwin",
        "version": "14.4.1"
      },
      "previous": {
        "hostname": "idr-2097-sonoma-14-9903"
      }
    }
  },
  "fields": {
    "event.created": [
      "2025-02-14T16:14:13.912Z"
    ],
    "@timestamp": [
      "2025-02-14T16:14:17.000Z"
    ]
  },
  "sort": [
    1739549657000
  ]
}
```

## Hardware

### Linux (Ubuntu 22.04)

<a name="Hardware-Linux-Stateful"></a>Stateful

```json
{
  "_index": "wazuh-states-inventory-hardware",
  "_id": "cf31a34a0991d005d4319476e20014d233c540e4",
  "_version": 1,
  "_score": 0,
  "_source": {
    "agent": {
      "id": "87676c24-26ea-4505-8618-e049b901f389",
      "name": "vm-ubuntu2204-agent",
      "groups": [],
      "type": "Endpoint",
      "version": "5.0.0",
      "host": {
        "architecture": "x86_64",
        "hostname": "vm-ubuntu2204-agent",
        "ip": [
          "10.0.2.15",
          "fe80::a00:27ff:fefc:450",
          "192.168.56.4",
          "fe80::a00:27ff:fe04:2d5b"
        ],
        "os": {
          "name": "Ubuntu",
          "type": "Linux",
          "version": "22.04.2 LTS (Jammy Jellyfish)"
        }
      }
    },
    "@timestamp": "2025-02-13T18:13:33.646Z",
    "host": {
      "cpu": {
        "cores": 4,
        "name": "AMD Ryzen 7 5800X 8-Core Processor",
        "speed": 3800
      },
      "memory": {
        "free": 3204636,
        "total": 4017720,
        "used": {
          "percentage": 21
        }
      }
    },
    "observer": {
      "serial_number": "0"
    }
  },
  "fields": {
    "@timestamp": [
      "2025-02-13T18:13:33.646Z"
    ]
  }
}
```


<a name="Hardware-Linux-Stateless"></a>Stateless

```json
{
  "_index": "wazuh-alerts-5.x-0001",
  "_id": "Tj-gAJUBUF68aH7Ejsoh",
  "_version": 1,
  "_score": 0,
  "_source": {
    "@timestamp": "2025-02-13T18:44:33Z",
    "agent": {
      "groups": [],
      "host": {
        "architecture": "x86_64",
        "hostname": "vm-ubuntu2204-agent",
        "ip": [
          "10.0.2.15",
          "fe80::a00:27ff:fefc:450",
          "192.168.56.4",
          "fe80::a00:27ff:fe04:2d5b"
        ],
        "os": {
          "name": "Ubuntu",
          "type": "Linux",
          "version": "22.04.2 LTS (Jammy Jellyfish)"
        }
      },
      "id": "87676c24-26ea-4505-8618-e049b901f389",
      "name": "vm-ubuntu2204-agent",
      "type": "Endpoint",
      "version": "5.0.0"
    },
    "event": {
      "action": "hardware-updated",
      "category": [
        "host"
      ],
      "changed_fields": [
        "host.memory.free"
      ],
      "collector": "hardware",
      "created": "2025-02-13T18:44:31.061Z",
      "module": "inventory",
      "reason": "Hardware changed",
      "type": [
        "change"
      ]
    },
    "host": {
      "cpu": {
        "cores": 4,
        "name": "AMD Ryzen 7 5800X 8-Core Processor",
        "speed": 3800
      },
      "memory": {
        "free": 3212952,
        "previous": {
          "free": 3212536
        },
        "total": 4017720,
        "used": {
          "percentage": 21
        }
      }
    },
    "observer": {
      "serial_number": "0"
    }
  },
  "fields": {
    "event.created": [
      "2025-02-13T18:44:31.061Z"
    ],
    "@timestamp": [
      "2025-02-13T18:44:33.000Z"
    ]
  },
  "sort": [
    0,
    null
  ]
}
```

### Windows (Windows 10)

<a name="Hardware-Windows-Stateful"></a>Stateful

```json
{
  "_index": "wazuh-states-inventory-hardware",
  "_id": "d7cb139bcc97a3df8e725a4d768b77096781dd26",
  "_version": 72,
  "_score": 0,
  "_source": {
    "agent": {
      "id": "51728df7-57d2-47b6-a065-a22995087bd7",
      "name": "DESKTOP-U8OHD3A",
      "groups": [],
      "type": "Endpoint",
      "version": "5.0.0",
      "host": {
        "architecture": "x86_64",
        "hostname": "DESKTOP-U8OHD3A",
        "ip": [
          "192.168.124.1",
          "fe80::4a97:2e71:39f4:21e5"
        ],
        "os": {
          "name": "Microsoft Windows 10 Home",
          "type": "Unknown",
          "version": "10.0.19045.5371"
        }
      }
    },
    "@timestamp": "2025-02-15T14:45:02.623Z",
    "host": {
      "cpu": {
        "cores": 8,
        "name": "AMD Ryzen 5 3500U with Radeon Vega Mobile Gfx  ",
        "speed": 2096
      },
      "memory": {
        "free": 8072164,
        "total": 18820708,
        "used": {
          "percentage": 57
        }
      }
    },
    "observer": {
      "serial_number": "L832NBCV006R8AMB"
    }
  },
  "fields": {
    "@timestamp": [
      "2025-02-15T14:45:02.623Z"
    ]
  }
}

```

<a name="Hardware-Windows-Stateless"></a>Stateless

```json
{
  "_index": "wazuh-alerts-5.x-0001",
  "_id": "s6XNFZUBJYGzKyMzzm6Q",
  "_version": 1,
  "_score": null,
  "_source": {
    "@timestamp": "2025-02-17T21:25:59Z",
    "agent": {
      "groups": [],
      "host": {
        "architecture": "x86_64",
        "hostname": "DESKTOP-U8OHD3A",
        "ip": [
          "192.168.124.1",
          "fe80::4a97:2e71:39f4:21e5"
        ],
        "os": {
          "name": "Microsoft Windows 10 Home",
          "type": "Unknown",
          "version": "10.0.19045.5487"
        }
      },
      "id": "4252f628-46c1-43c4-a184-f19887e768fc",
      "name": "DESKTOP-U8OHD3A",
      "type": "Endpoint",
      "version": "5.0.0"
    },
    "event": {
      "action": "hardware-updated",
      "category": [
        "host"
      ],
      "changed_fields": [
        "host.memory.free",
        "host.memory.used.percentage"
      ],
      "collector": "hardware",
      "created": "2025-02-17T21:25:55.593Z",
      "module": "inventory",
      "reason": "Hardware changed",
      "type": [
        "change"
      ]
    },
    "host": {
      "cpu": {
        "cores": 8,
        "name": "AMD Ryzen 5 3500U with Radeon Vega Mobile Gfx  ",
        "speed": 2096
      },
      "memory": {
        "free": 11308888,
        "previous": {
          "free": 11227004
        },
        "total": 18820708,
        "used": {
          "percentage": 39,
          "previous": {
            "percentage": 40
          }
        }
      }
    },
    "observer": {
      "serial_number": "L832NBCV006R8AMB"
    }
  },
  "fields": {
    "event.created": [
      "2025-02-17T21:25:55.593Z"
    ],
    "@timestamp": [
      "2025-02-17T21:25:59.000Z"
    ]
  },
  "highlight": {
    "event.action": [
      "@opensearch-dashboards-highlighted-field@hardware-updated@/opensearch-dashboards-highlighted-field@"
    ]
  },
  "sort": [
    1739827559000
  ]
}

```

### Macos (macOS Sonoma 14.4.1)

<a name="Hardware-Macos-Stateful"></a>Stateful

```json
{
  "_index": "wazuh-states-inventory-hardware",
  "_id": "1846d3f6b0829b0af30bdf46f2ba52b193b0e54b",
  "_version": 1,
  "_score": 0,
  "_source": {
    "agent": {
      "id": "e7aadfe0-1971-4351-9767-34a330be3c4e",
      "name": "idr-2097-sonoma-14-9903",
      "groups": [],
      "type": "Endpoint",
      "version": "5.0.0",
      "host": {
        "architecture": "x86_64",
        "hostname": "idr-2097-sonoma-14-9903",
        "ip": [
          "10.211.55.103",
          "fe80::5c:92d1:bd06:96e5"
        ],
        "os": {
          "name": "macOS",
          "type": "Darwin",
          "version": "14.4.1"
        }
      }
    },
    "@timestamp": "2025-02-14T15:38:37.835Z",
    "host": {
      "cpu": {
        "cores": 2,
        "name": "Intel(R) Core(TM) i7-8700B CPU @ 3.20GHz",
        "speed": 3192
      },
      "memory": {
        "free": 1239552,
        "total": 4194304,
        "used": {
          "percentage": 71
        }
      }
    },
    "observer": {
      "serial_number": "H2WF603JPJJ9"
    }
  },
  "fields": {
    "@timestamp": [
      "2025-02-14T15:38:37.835Z"
    ]
  }
}
```

<a name="Hardware-Macos-Stateless"></a>Stateless

```json
{
  "_index": "wazuh-alerts-5.x-0001",
  "_id": "h1A1BZUBA1q-qsf1H5oG",
  "_version": 1,
  "_score": null,
  "_source": {
    "@timestamp": "2025-02-14T16:05:21Z",
    "agent": {
      "groups": [],
      "host": {
        "architecture": "x86_64",
        "hostname": "idr-2097-sonoma-14-9903",
        "ip": [
          "10.211.55.103",
          "fe80::5c:92d1:bd06:96e5"
        ],
        "os": {
          "name": "macOS",
          "type": "Darwin",
          "version": "14.4.1"
        }
      },
      "id": "e7aadfe0-1971-4351-9767-34a330be3c4e",
      "name": "idr-2097-sonoma-14-9903",
      "type": "Endpoint",
      "version": "5.0.0"
    },
    "event": {
      "action": "hardware-updated",
      "category": [
        "host"
      ],
      "changed_fields": [
        "host.memory.free"
      ],
      "collector": "hardware",
      "created": "2025-02-14T16:05:11.271Z",
      "module": "inventory",
      "reason": "Hardware changed",
      "type": [
        "change"
      ]
    },
    "host": {
      "cpu": {
        "cores": 2,
        "name": "Intel(R) Core(TM) i7-8700B CPU @ 3.20GHz",
        "speed": 3192
      },
      "memory": {
        "free": 1224236,
        "previous": {
          "free": 1239552
        },
        "total": 4194304,
        "used": {
          "percentage": 71
        }
      }
    },
    "observer": {
      "serial_number": "H2WF603JPJJ9"
    }
  },
  "fields": {
    "event.created": [
      "2025-02-14T16:05:11.271Z"
    ],
    "@timestamp": [
      "2025-02-14T16:05:21.000Z"
    ]
  },
  "sort": [
    1739549121000
  ]
}
```


## Packages

### Linux (Ubuntu 22.04)

<a name="Packages-Linux-Stateful"></a>Stateful

```json
{
  "_index": "wazuh-states-inventory-packages",
  "_id": "f1262fa48290d36d8ca48a265bad8a9cb07f6726",
  "_version": 1,
  "_score": 0,
  "_source": {
    "agent": {
      "id": "87676c24-26ea-4505-8618-e049b901f389",
      "name": "vm-ubuntu2204-agent",
      "groups": [],
      "type": "Endpoint",
      "version": "5.0.0",
      "host": {
        "architecture": "x86_64",
        "hostname": "vm-ubuntu2204-agent",
        "ip": [
          "10.0.2.15",
          "fe80::a00:27ff:fefc:450",
          "192.168.56.4",
          "fe80::a00:27ff:fe04:2d5b"
        ],
        "os": {
          "name": "Ubuntu",
          "type": "Linux",
          "version": "22.04.2 LTS (Jammy Jellyfish)"
        }
      }
    },
    "@timestamp": "2025-02-13T18:13:33.646Z",
    "package": {
      "architecture": "amd64",
      "description": "Wazuh helps you to gain security visibility into your infrastructure",
      "installed": null,
      "name": "wazuh-agent",
      "path": null,
      "size": 16525312,
      "type": "deb",
      "version": "5.0.0-0"
    }
  },
  "fields": {
    "@timestamp": [
      "2025-02-13T18:13:33.646Z"
    ]
  },
  "sort": [
    0,
    "wazuh-agent"
  ]
}
```


<a name="Packages-Linux-Stateless"></a>Stateless

```json
{
  "_index": "wazuh-alerts-5.x-0001",
  "_id": "1T_EAJUBUF68aH7ESMqx",
  "_version": 1,
  "_score": 0,
  "_source": {
    "@timestamp": "2025-02-13T19:23:35Z",
    "agent": {
      "groups": [],
      "host": {
        "architecture": "x86_64",
        "hostname": "vm-ubuntu2204-agent",
        "ip": [
          "10.0.2.15",
          "fe80::a00:27ff:fefc:450",
          "192.168.56.4",
          "fe80::a00:27ff:fe04:2d5b"
        ],
        "os": {
          "name": "Ubuntu",
          "type": "Linux",
          "version": "22.04.2 LTS (Jammy Jellyfish)"
        }
      },
      "id": "87676c24-26ea-4505-8618-e049b901f389",
      "name": "vm-ubuntu2204-agent",
      "type": "Endpoint",
      "version": "5.0.0"
    },
    "event": {
      "action": "package-installed",
      "category": [
        "package"
      ],
      "collector": "packages",
      "created": "2025-02-13T19:23:32.639Z",
      "module": "inventory",
      "reason": "Package cmake (version 3.22.1-1ubuntu1.22.04.2) was installed",
      "type": [
        "installation"
      ]
    },
    "package": {
      "architecture": "amd64",
      "description": "cross-platform, open-source make system",
      "installed": null,
      "name": "cmake",
      "path": null,
      "size": 21239808,
      "type": "deb",
      "version": "3.22.1-1ubuntu1.22.04.2"
    }
  },
  "fields": {
    "event.created": [
      "2025-02-13T19:23:32.639Z"
    ],
    "@timestamp": [
      "2025-02-13T19:23:35.000Z"
    ]
  },
  "highlight": {
    "event.action": [
      "@opensearch-dashboards-highlighted-field@package-installed@/opensearch-dashboards-highlighted-field@"
    ]
  },
  "sort": [
    0,
    "cmake"
  ]
}
```

### Windows (Windows 10)

<a name="Packages-Windows-Stateful"></a>Stateful

```json
{
  "_index": "wazuh-states-inventory-packages",
  "_id": "0f76892f9c69a3de7696e29ebdf2a7e1a099eab1",
  "_version": 1,
  "_score": 0,
  "_source": {
    "agent": {
      "id": "51728df7-57d2-47b6-a065-a22995087bd7",
      "name": "DESKTOP-U8OHD3A",
      "groups": [],
      "type": "Endpoint",
      "version": "5.0.0",
      "host": {
        "architecture": "x86_64",
        "hostname": "DESKTOP-U8OHD3A",
        "ip": [
          "192.168.124.1",
          "fe80::4a97:2e71:39f4:21e5"
        ],
        "os": {
          "name": "Microsoft Windows 10 Home",
          "type": "Unknown",
          "version": "10.0.19045.5371"
        }
      }
    },
    "@timestamp": "2025-02-14T23:09:42.160Z",
    "package": {
      "architecture": "x86_64",
      "description": null,
      "installed": "2021-05-10T20:21:46.000Z",
      "name": "Sublime Text 3",
      "path": "C:\\Program Files\\Sublime Text 3\\",
      "size": null,
      "type": "win",
      "version": null
    }
  },
  "fields": {
    "@timestamp": [
      "2025-02-14T23:09:42.160Z"
    ],
    "package.installed": [
      "2021-05-10T20:21:46.000Z"
    ]
  },
  "highlight": {
    "package.name": [
      "@opensearch-dashboards-highlighted-field@Sublime Text 3@/opensearch-dashboards-highlighted-field@"
    ]
  }
}
```

<a name="Packages-Windows-Stateless"></a>Stateless

```json
{
  "_index": "wazuh-alerts-5.x-0001",
  "_id": "QqUFFpUBJYGzKyMz73Zm",
  "_version": 1,
  "_score": null,
  "_source": {
    "@timestamp": "2025-02-17T22:27:18Z",
    "agent": {
      "groups": [],
      "host": {
        "architecture": "x86_64",
        "hostname": "DESKTOP-U8OHD3A",
        "ip": [
          "192.168.124.1",
          "fe80::4a97:2e71:39f4:21e5"
        ],
        "os": {
          "name": "Microsoft Windows 10 Home",
          "type": "Unknown",
          "version": "10.0.19045.5487"
        }
      },
      "id": "4252f628-46c1-43c4-a184-f19887e768fc",
      "name": "DESKTOP-U8OHD3A",
      "type": "Endpoint",
      "version": "5.0.0"
    },
    "event": {
      "action": "package-installed",
      "category": [
        "package"
      ],
      "collector": "packages",
      "created": "2025-02-17T22:27:16.455Z",
      "module": "inventory",
      "reason": "Package i686 (version ) was installed",
      "type": [
        "installation"
      ]
    },
    "package": {
      "architecture": "i686",
      "description": null,
      "installed": "2020-11-19T03:08:54.000Z",
      "name": "i686",
      "path": null,
      "size": null,
      "type": "win",
      "version": null
    }
  },
  "fields": {
    "event.created": [
      "2025-02-17T22:27:16.455Z"
    ],
    "package.installed": [
      "2020-11-19T03:08:54.000Z"
    ],
    "@timestamp": [
      "2025-02-17T22:27:18.000Z"
    ]
  },
  "highlight": {
    "event.action": [
      "@opensearch-dashboards-highlighted-field@package-installed@/opensearch-dashboards-highlighted-field@"
    ]
  },
  "sort": [
    1739831238000
  ]
}
```

### Macos (macOS Sonoma 14.4.1)

<a name="Packages-Macos-Stateful"></a>Stateful

```json
{
  "_index": "wazuh-states-inventory-packages",
  "_id": "f97ec430478a44abaf20b219e2d5d52d5a5d4a88",
  "_version": 1,
  "_score": 0,
  "_source": {
    "agent": {
      "id": "e7aadfe0-1971-4351-9767-34a330be3c4e",
      "name": "idr-2097-sonoma-14-9903",
      "groups": [],
      "type": "Endpoint",
      "version": "5.0.0",
      "host": {
        "architecture": "x86_64",
        "hostname": "idr-2097-sonoma-14-9903",
        "ip": [
          "10.211.55.103",
          "fe80::5c:92d1:bd06:96e5"
        ],
        "os": {
          "name": "macOS",
          "type": "Darwin",
          "version": "14.4.1"
        }
      }
    },
    "@timestamp": "2025-02-14T15:38:37.835Z",
    "package": {
      "architecture": null,
      "description": "com.apple.siri.launcher",
      "installed": null,
      "name": "Siri",
      "path": "/System/Applications/Siri.app/Contents/Info.plist",
      "size": null,
      "type": "pkg",
      "version": "1.0"
    }
  },
  "fields": {
    "@timestamp": [
      "2025-02-14T15:38:37.835Z"
    ]
  }
}
```

<a name="Packages-Macos-Stateless"></a>Stateless

```json
{
  "_index": "wazuh-alerts-5.x-0001",
  "_id": "5lBFBZUBA1q-qsf1sZoG",
  "_version": 1,
  "_score": null,
  "_source": {
    "@timestamp": "2025-02-14T16:23:25Z",
    "agent": {
      "groups": [],
      "host": {
        "architecture": "x86_64",
        "hostname": "idr-2097-sonoma-14-9903",
        "ip": [
          "10.211.55.103",
          "fe80::5c:92d1:bd06:96e5"
        ],
        "os": {
          "name": "macOS",
          "type": "Darwin",
          "version": "14.4.1"
        }
      },
      "id": "e7aadfe0-1971-4351-9767-34a330be3c4e",
      "name": "idr-2097-sonoma-14-9903",
      "type": "Endpoint",
      "version": "5.0.0"
    },
    "event": {
      "action": "package-installed",
      "category": [
        "package"
      ],
      "collector": "packages",
      "created": "2025-02-14T16:23:17.559Z",
      "module": "inventory",
      "reason": "Package pip (version 21.2.4) was installed",
      "type": [
        "installation"
      ]
    },
    "package": {
      "architecture": null,
      "description": null,
      "installed": null,
      "name": "pip",
      "path": "/Library/Developer/CommandLineTools/Library/Frameworks/Python3.framework/Versions/3.9/lib/python3.9/site-packages/pip-21.2.4.dist-info/METADATA",
      "size": null,
      "type": "pypi",
      "version": "21.2.4"
    }
  },
  "fields": {
    "event.created": [
      "2025-02-14T16:23:17.559Z"
    ],
    "@timestamp": [
      "2025-02-14T16:23:25.000Z"
    ]
  },
  "highlight": {
    "event.action": [
      "@opensearch-dashboards-highlighted-field@package-installed@/opensearch-dashboards-highlighted-field@"
    ]
  },
  "sort": [
    1739550205000
  ]
}
```

## Processes

### Linux (Ubuntu 22.04)

<a name="Processes-Linux-Stateful"></a>Stateful

```json
{
  "_index": "wazuh-states-inventory-processes",
  "_id": "6f6b2d83a697c9c9103dfce5c05c1fd56ccfa37b",
  "_version": 1,
  "_score": 0,
  "_source": {
    "agent": {
      "id": "87676c24-26ea-4505-8618-e049b901f389",
      "name": "vm-ubuntu2204-agent",
      "groups": [],
      "type": "Endpoint",
      "version": "5.0.0",
      "host": {
        "architecture": "x86_64",
        "hostname": "vm-ubuntu2204-agent",
        "ip": [
          "10.0.2.15",
          "fe80::a00:27ff:fefc:450",
          "192.168.56.4",
          "fe80::a00:27ff:fe04:2d5b"
        ],
        "os": {
          "name": "Ubuntu",
          "type": "Linux",
          "version": "22.04.2 LTS (Jammy Jellyfish)"
        }
      }
    },
    "@timestamp": "2025-02-13T18:34:54.882Z",
    "process": {
      "args": null,
      "command_line": "/usr/lib/snapd/snapd",
      "group": {
        "id": "root"
      },
      "name": "snapd",
      "parent": {
        "pid": 1
      },
      "pid": "677",
      "real_group": {
        "id": "root"
      },
      "real_user": {
        "id": "root"
      },
      "saved_group": {
        "id": "root"
      },
      "saved_user": {
        "id": "root"
      },
      "start": 1739462452,
      "thread": {
        "id": 677
      },
      "tty": {
        "char_device": {
          "major": 0
        }
      },
      "user": {
        "id": "root"
      }
    }
  },
  "fields": {
    "@timestamp": [
      "2025-02-13T18:34:54.882Z"
    ],
    "process.start": [
      "1970-01-21T03:11:02.452Z"
    ]
  }
}
```


<a name="Processes-Linux-Stateless"></a>Stateless

```json
{
  "_index": "wazuh-alerts-5.x-0001",
  "_id": "Tz-gAJUBUF68aH7Ejsoh",
  "_version": 1,
  "_score": 0,
  "_source": {
    "@timestamp": "2025-02-13T18:44:33Z",
    "agent": {
      "groups": [],
      "host": {
        "architecture": "x86_64",
        "hostname": "vm-ubuntu2204-agent",
        "ip": [
          "10.0.2.15",
          "fe80::a00:27ff:fefc:450",
          "192.168.56.4",
          "fe80::a00:27ff:fe04:2d5b"
        ],
        "os": {
          "name": "Ubuntu",
          "type": "Linux",
          "version": "22.04.2 LTS (Jammy Jellyfish)"
        }
      },
      "id": "87676c24-26ea-4505-8618-e049b901f389",
      "name": "vm-ubuntu2204-agent",
      "type": "Endpoint",
      "version": "5.0.0"
    },
    "event": {
      "action": "process-updated",
      "category": [
        "process"
      ],
      "changed_fields": [
        "process.name"
      ],
      "collector": "processes",
      "created": "2025-02-13T18:44:31.061Z",
      "module": "inventory",
      "reason": "Process kworker/2:2-mm_ (PID: kworker/2:2-mm_) was updated",
      "type": [
        "change"
      ]
    },
    "process": {
      "args": null,
      "command_line": null,
      "group": {
        "id": "root"
      },
      "name": "kworker/2:2-mm_",
      "parent": {
        "pid": 2
      },
      "pid": "351",
      "previous": {
        "name": "kworker/2:2-eve"
      },
      "real_group": {
        "id": "root"
      },
      "real_user": {
        "id": "root"
      },
      "saved_group": {
        "id": "root"
      },
      "saved_user": {
        "id": "root"
      },
      "start": 1739462451,
      "thread": {
        "id": 351
      },
      "tty": {
        "char_device": {
          "major": 0
        }
      },
      "user": {
        "id": "root"
      }
    }
  },
  "fields": {
    "event.created": [
      "2025-02-13T18:44:31.061Z"
    ],
    "process.start": [
      "1970-01-21T03:11:02.451Z"
    ],
    "@timestamp": [
      "2025-02-13T18:44:33.000Z"
    ]
  },
  "sort": [
    0,
    null
  ]
}
```

### Windows (Windows 10)

<a name="Processes-Windows-Stateful"></a>Stateful

```json
{
  "_index": "wazuh-states-inventory-processes",
  "_id": "1c557f5fc1044793361126534423748bc16435ba",
  "_version": 1,
  "_score": 0,
  "_source": {
    "agent": {
      "id": "51728df7-57d2-47b6-a065-a22995087bd7",
      "name": "DESKTOP-U8OHD3A",
      "groups": [],
      "type": "Endpoint",
      "version": "5.0.0",
      "host": {
        "architecture": "x86_64",
        "hostname": "DESKTOP-U8OHD3A",
        "ip": [
          "192.168.124.1",
          "fe80::4a97:2e71:39f4:21e5"
        ],
        "os": {
          "name": "Microsoft Windows 10 Home",
          "type": "Unknown",
          "version": "10.0.19045.5371"
        }
      }
    },
    "@timestamp": "2025-02-14T23:09:42.160Z",
    "process": {
      "args": null,
      "command_line": "C:\\Windows\\System32\\DriverStore\\FileRepository\\realtekservice.inf_amd64_6903f1a9d3b68dab\\RtkAudUService64.exe",
      "group": {
        "id": null
      },
      "name": "RtkAudUService64.exe",
      "parent": {
        "pid": 1132
      },
      "pid": "6588",
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
      "start": 1737771017,
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
  },
  "fields": {
    "@timestamp": [
      "2025-02-14T23:09:42.160Z"
    ],
    "process.start": [
      "1970-01-21T02:42:51.017Z"
    ]
  },
  "highlight": {
    "process.name": [
      "@opensearch-dashboards-highlighted-field@RtkAudUService64.exe@/opensearch-dashboards-highlighted-field@"
    ]
  }
}
```

<a name="Processes-Windows-Stateless"></a>Stateless

```json
{
  "_index": "wazuh-alerts-5.x-0001",
  "_id": "jqW5FZUBJYGzKyMzkWsh",
  "_version": 1,
  "_score": null,
  "_source": {
    "@timestamp": "2025-02-17T21:03:57Z",
    "agent": {
      "groups": [],
      "host": {
        "architecture": "x86_64",
        "hostname": "DESKTOP-U8OHD3A",
        "ip": [
          "192.168.124.1",
          "fe80::4a97:2e71:39f4:21e5"
        ],
        "os": {
          "name": "Microsoft Windows 10 Home",
          "type": "Unknown",
          "version": "10.0.19045.5487"
        }
      },
      "id": "4252f628-46c1-43c4-a184-f19887e768fc",
      "name": "DESKTOP-U8OHD3A",
      "type": "Endpoint",
      "version": "5.0.0"
    },
    "event": {
      "action": "process-updated",
      "category": [
        "process"
      ],
      "changed_fields": [
        "process.command_line",
        "process.name",
        "process.start",
        "process.parent.pid"
      ],
      "collector": "processes",
      "created": "2025-02-17T21:03:47.940Z",
      "module": "inventory",
      "reason": "Process chrome.exe (PID: chrome.exe) was updated",
      "type": [
        "change"
      ]
    },
    "process": {
      "args": null,
      "command_line": "C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe",
      "group": {
        "id": null
      },
      "name": "chrome.exe",
      "parent": {
        "pid": 14432,
        "previous": {
          "pid": 12236
        }
      },
      "pid": "10960",
      "previous": {
        "command_line": "C:\\Windows\\System32\\net.exe",
        "name": "net.exe",
        "start": 1739826166
      },
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
      "start": 1739826227,
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
  },
  "fields": {
    "event.created": [
      "2025-02-17T21:03:47.940Z"
    ],
    "process.start": [
      "1970-01-21T03:17:06.227Z"
    ],
    "@timestamp": [
      "2025-02-17T21:03:57.000Z"
    ]
  },
  "highlight": {
    "event.action": [
      "@opensearch-dashboards-highlighted-field@process-updated@/opensearch-dashboards-highlighted-field@"
    ]
  },
  "sort": [
    1739826237000
  ]
}
```

### Macos (macOS Sonoma 14.4.1)

<a name="Processes-Macos-Stateful"></a>Stateful

```json
{
  "_index": "wazuh-states-inventory-processes",
  "_id": "b086d093f40ed071cffc4f86d661d71c9446542f",
  "_version": 1,
  "_score": 0,
  "_source": {
    "agent": {
      "id": "e7aadfe0-1971-4351-9767-34a330be3c4e",
      "name": "idr-2097-sonoma-14-9903",
      "groups": [],
      "type": "Endpoint",
      "version": "5.0.0",
      "host": {
        "architecture": "x86_64",
        "hostname": "idr-2097-sonoma-14-9903",
        "ip": [
          "10.211.55.103",
          "fe80::5c:92d1:bd06:96e5"
        ],
        "os": {
          "name": "macOS",
          "type": "Darwin",
          "version": "14.4.1"
        }
      }
    },
    "@timestamp": "2025-02-14T15:38:37.835Z",
    "process": {
      "args": null,
      "command_line": null,
      "group": {
        "id": null
      },
      "name": "wazuh-agent",
      "parent": {
        "pid": 1
      },
      "pid": "1898",
      "real_group": {
        "id": "wheel"
      },
      "real_user": {
        "id": "root"
      },
      "saved_group": {
        "id": null
      },
      "saved_user": {
        "id": null
      },
      "start": 1739547517,
      "thread": {
        "id": null
      },
      "tty": {
        "char_device": {
          "major": null
        }
      },
      "user": {
        "id": "root"
      }
    }
  },
  "fields": {
    "@timestamp": [
      "2025-02-14T15:38:37.835Z"
    ],
    "process.start": [
      "1970-01-21T03:12:27.517Z"
    ]
  }
}
```

<a name="Processes-Macos-Stateless"></a>Stateless

```json
{
  "_index": "wazuh-alerts-5.x-0001",
  "_id": "ilA1BZUBA1q-qsf1H5oG",
  "_version": 1,
  "_score": null,
  "_source": {
    "@timestamp": "2025-02-14T16:05:21Z",
    "agent": {
      "groups": [],
      "host": {
        "architecture": "x86_64",
        "hostname": "idr-2097-sonoma-14-9903",
        "ip": [
          "10.211.55.103",
          "fe80::5c:92d1:bd06:96e5"
        ],
        "os": {
          "name": "macOS",
          "type": "Darwin",
          "version": "14.4.1"
        }
      },
      "id": "e7aadfe0-1971-4351-9767-34a330be3c4e",
      "name": "idr-2097-sonoma-14-9903",
      "type": "Endpoint",
      "version": "5.0.0"
    },
    "event": {
      "action": "process-stopped",
      "category": [
        "process"
      ],
      "collector": "processes",
      "created": "2025-02-14T16:05:11.271Z",
      "module": "inventory",
      "reason": "Process wazuh-agent (PID: wazuh-agent) was stopped",
      "type": [
        "end"
      ]
    },
    "process": {
      "args": null,
      "command_line": null,
      "group": {
        "id": null
      },
      "name": "wazuh-agent",
      "parent": {
        "pid": 1
      },
      "pid": "1898",
      "real_group": {
        "id": "wheel"
      },
      "real_user": {
        "id": "root"
      },
      "saved_group": {
        "id": null
      },
      "saved_user": {
        "id": null
      },
      "start": 1739547517,
      "thread": {
        "id": null
      },
      "tty": {
        "char_device": {
          "major": null
        }
      },
      "user": {
        "id": "root"
      }
    }
  },
  "fields": {
    "event.created": [
      "2025-02-14T16:05:11.271Z"
    ],
    "process.start": [
      "1970-01-21T03:12:27.517Z"
    ],
    "@timestamp": [
      "2025-02-14T16:05:21.000Z"
    ]
  },
  "sort": [
    1739549121000
  ]
}
```

## Networks

### Linux (Ubuntu 22.04)

<a name="Networks-Linux-Stateful"></a>Stateful

```json
{
  "_index": "wazuh-states-inventory-networks",
  "_id": "4d345de9adcf341e5cdc530c596d0955f7415e5c",
  "_version": 1,
  "_score": 0,
  "_source": {
    "agent": {
      "id": "87676c24-26ea-4505-8618-e049b901f389",
      "name": "vm-ubuntu2204-agent",
      "groups": [],
      "type": "Endpoint",
      "version": "5.0.0",
      "host": {
        "architecture": "x86_64",
        "hostname": "vm-ubuntu2204-agent",
        "ip": [
          "10.0.2.15",
          "fe80::a00:27ff:fefc:450",
          "192.168.56.4",
          "fe80::a00:27ff:fe04:2d5b"
        ],
        "os": {
          "name": "Ubuntu",
          "type": "Linux",
          "version": "22.04.2 LTS (Jammy Jellyfish)"
        }
      }
    },
    "@timestamp": "2025-02-13T18:13:33.646Z",
    "host": {
      "ip": [
        "10.0.2.15"
      ],
      "mac": "08:00:27:fc:04:50",
      "network": {
        "egress": {
          "bytes": 224796,
          "drops": 0,
          "errors": 0,
          "packets": 1627
        },
        "ingress": {
          "bytes": 265579,
          "drops": 0,
          "errors": 0,
          "packets": 2445
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
        "10.0.2.255"
      ],
      "dhcp": null,
      "gateway": [
        "10.0.2.2"
      ],
      "metric": "100",
      "netmask": [
        "255.255.255.0"
      ],
      "type": "ipv4"
    },
    "observer": {
      "ingress": {
        "interface": {
          "alias": null,
          "name": "eth0"
        }
      }
    }
  },
  "fields": {
    "@timestamp": [
      "2025-02-13T18:13:33.646Z"
    ]
  }
}
```


<a name="Networks-Linux-Stateless"></a>Stateless

```json
{
  "_index": "wazuh-alerts-5.x-0001",
  "_id": "VD-gAJUBUF68aH7Ejsoh",
  "_version": 1,
  "_score": 0,
  "_source": {
    "@timestamp": "2025-02-13T18:44:33Z",
    "agent": {
      "groups": [],
      "host": {
        "architecture": "x86_64",
        "hostname": "vm-ubuntu2204-agent",
        "ip": [
          "10.0.2.15",
          "fe80::a00:27ff:fefc:450",
          "192.168.56.4",
          "fe80::a00:27ff:fe04:2d5b"
        ],
        "os": {
          "name": "Ubuntu",
          "type": "Linux",
          "version": "22.04.2 LTS (Jammy Jellyfish)"
        }
      },
      "id": "87676c24-26ea-4505-8618-e049b901f389",
      "name": "vm-ubuntu2204-agent",
      "type": "Endpoint",
      "version": "5.0.0"
    },
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
      "collector": "networks",
      "created": "2025-02-13T18:44:31.061Z",
      "module": "inventory",
      "reason": "Network interface eth1 updated",
      "type": [
        "change"
      ]
    },
    "host": {
      "ip": [
        "fe80::a00:27ff:fe04:2d5b"
      ],
      "mac": "08:00:27:04:2d:5b",
      "network": {
        "egress": {
          "bytes": 503328,
          "drops": 0,
          "errors": 0,
          "packets": 979,
          "previous": {
            "bytes": 469981,
            "packets": 901
          }
        },
        "ingress": {
          "bytes": 492597,
          "drops": 0,
          "errors": 0,
          "packets": 3255,
          "previous": {
            "bytes": 466407,
            "packets": 3150
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
      "dhcp": null,
      "gateway": [],
      "metric": null,
      "netmask": [
        "ffff:ffff:ffff:ffff::"
      ],
      "type": "ipv6"
    },
    "observer": {
      "ingress": {
        "interface": {
          "alias": null,
          "name": "eth1"
        }
      }
    }
  },
  "fields": {
    "event.created": [
      "2025-02-13T18:44:31.061Z"
    ],
    "@timestamp": [
      "2025-02-13T18:44:33.000Z"
    ]
  },
  "highlight": {
    "event.action": [
      "@opensearch-dashboards-highlighted-field@network-interface-updated@/opensearch-dashboards-highlighted-field@"
    ]
  },
  "sort": [
    0,
    null
  ]
}
```

### Windows (Windows 10)

<a name="Networks-Windows-Stateful"></a>Stateful

```json
{
  "_index": "wazuh-states-inventory-networks",
  "_id": "42a41e9ee314e121754ae4fda87826aa8db8ffcd",
  "_version": 1,
  "_score": 0,
  "_source": {
    "agent": {
      "id": "51728df7-57d2-47b6-a065-a22995087bd7",
      "name": "DESKTOP-U8OHD3A",
      "groups": [],
      "type": "Endpoint",
      "version": "5.0.0",
      "host": {
        "architecture": "x86_64",
        "hostname": "DESKTOP-U8OHD3A",
        "ip": [
          "192.168.124.1",
          "fe80::4a97:2e71:39f4:21e5"
        ],
        "os": {
          "name": "Microsoft Windows 10 Home",
          "type": "Unknown",
          "version": "10.0.19045.5371"
        }
      }
    },
    "@timestamp": "2025-02-14T23:09:42.160Z",
    "host": {
      "ip": [
        "169.254.6.76"
      ],
      "mac": "0a:00:27:00:00:06",
      "network": {
        "egress": {
          "bytes": 21765369,
          "drops": 0,
          "errors": 0,
          "packets": 210193
        },
        "ingress": {
          "bytes": 12139677,
          "drops": 0,
          "errors": 0,
          "packets": 141535
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
        "169.254.255.255"
      ],
      "dhcp": "enabled",
      "gateway": [],
      "metric": "25",
      "netmask": [
        "255.255.0.0"
      ],
      "type": "ipv4"
    },
    "observer": {
      "ingress": {
        "interface": {
          "alias": "VirtualBox Host-Only Ethernet Adapter",
          "name": "VirtualBox Host-Only Network"
        }
      }
    }
  },
  "fields": {
    "@timestamp": [
      "2025-02-14T23:09:42.160Z"
    ]
  }
}
```

<a name="Networks-Windows-Stateless"></a>Stateless

```json
{
  "_index": "wazuh-alerts-5.x-0001",
  "_id": "FKXQFZUBJYGzKyMzjm9d",
  "_version": 1,
  "_score": null,
  "_source": {
    "@timestamp": "2025-02-17T21:29:04Z",
    "agent": {
      "groups": [],
      "host": {
        "architecture": "x86_64",
        "hostname": "DESKTOP-U8OHD3A",
        "ip": [
          "192.168.124.1",
          "fe80::4a97:2e71:39f4:21e5"
        ],
        "os": {
          "name": "Microsoft Windows 10 Home",
          "type": "Unknown",
          "version": "10.0.19045.5487"
        }
      },
      "id": "4252f628-46c1-43c4-a184-f19887e768fc",
      "name": "DESKTOP-U8OHD3A",
      "type": "Endpoint",
      "version": "5.0.0"
    },
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
      "collector": "networks",
      "created": "2025-02-17T21:28:56.509Z",
      "module": "inventory",
      "reason": "Network interface Wi-Fi updated",
      "type": [
        "change"
      ]
    },
    "host": {
      "ip": [
        "2803:9800:9882:42a3:a6e3:7f69:3201:8936"
      ],
      "mac": "d8:c0:a6:48:13:c3",
      "network": {
        "egress": {
          "bytes": 1241510241,
          "drops": 0,
          "errors": 0,
          "packets": 3417741,
          "previous": {
            "bytes": 1240950194,
            "packets": 3414337
          }
        },
        "ingress": {
          "bytes": 3823892959,
          "drops": 0,
          "errors": 0,
          "packets": 5186278,
          "previous": {
            "bytes": 3814205060,
            "packets": 5177945
          }
        }
      }
    },
    "interface": {
      "mtu": 1500,
      "state": "up",
      "type": "wireless"
    },
    "network": {
      "broadcast": [],
      "dhcp": "enabled",
      "gateway": [
        "fe80::1,192.168.100.1"
      ],
      "metric": "55",
      "netmask": [
        "ffff:ffff:ffff:ffff::"
      ],
      "type": "ipv6"
    },
    "observer": {
      "ingress": {
        "interface": {
          "alias": "Qualcomm Atheros QCA9377 Wireless Network Adapter",
          "name": "Wi-Fi"
        }
      }
    }
  },
  "fields": {
    "event.created": [
      "2025-02-17T21:28:56.509Z"
    ],
    "@timestamp": [
      "2025-02-17T21:29:04.000Z"
    ]
  },
  "highlight": {
    "event.action": [
      "@opensearch-dashboards-highlighted-field@network-interface-updated@/opensearch-dashboards-highlighted-field@"
    ]
  },
  "sort": [
    1739827744000
  ]
}

```

### Macos (macOS Sonoma 14.4.1)

<a name="Networks-Macos-Stateful"></a>Stateful

```json
{
  "_index": "wazuh-states-inventory-networks",
  "_id": "2d0400d16e0128ca27cd2d0453bbde4ff5b09bc3",
  "_version": 1,
  "_score": 0,
  "_source": {
    "agent": {
      "id": "e7aadfe0-1971-4351-9767-34a330be3c4e",
      "name": "idr-2097-sonoma-14-9903",
      "groups": [],
      "type": "Endpoint",
      "version": "5.0.0",
      "host": {
        "architecture": "x86_64",
        "hostname": "idr-2097-sonoma-14-9903",
        "ip": [
          "10.211.55.103",
          "fe80::5c:92d1:bd06:96e5"
        ],
        "os": {
          "name": "macOS",
          "type": "Darwin",
          "version": "14.4.1"
        }
      }
    },
    "@timestamp": "2025-02-14T15:38:37.835Z",
    "host": {
      "ip": [
        "10.211.55.103"
      ],
      "mac": "00:1c:42:ce:16:50",
      "network": {
        "egress": {
          "bytes": 496640,
          "drops": 0,
          "errors": 0,
          "packets": 4869
        },
        "ingress": {
          "bytes": 5767168,
          "drops": 0,
          "errors": 0,
          "packets": 6478
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
        "10.211.55.255"
      ],
      "dhcp": null,
      "gateway": [
        "10.211.55.1"
      ],
      "metric": null,
      "netmask": [
        "255.255.255.0"
      ],
      "type": "ipv4"
    },
    "observer": {
      "ingress": {
        "interface": {
          "alias": null,
          "name": "en0"
        }
      }
    }
  },
  "fields": {
    "@timestamp": [
      "2025-02-14T15:38:37.835Z"
    ]
  }
}
```

<a name="Networks-Macos-Stateless"></a>Stateless

```json
{
  "_index": "wazuh-alerts-5.x-0001",
  "_id": "jVA1BZUBA1q-qsf1H5oG",
  "_version": 1,
  "_score": null,
  "_source": {
    "@timestamp": "2025-02-14T16:05:21Z",
    "agent": {
      "groups": [],
      "host": {
        "architecture": "x86_64",
        "hostname": "idr-2097-sonoma-14-9903",
        "ip": [
          "10.211.55.103",
          "fe80::5c:92d1:bd06:96e5"
        ],
        "os": {
          "name": "macOS",
          "type": "Darwin",
          "version": "14.4.1"
        }
      },
      "id": "e7aadfe0-1971-4351-9767-34a330be3c4e",
      "name": "idr-2097-sonoma-14-9903",
      "type": "Endpoint",
      "version": "5.0.0"
    },
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
      "collector": "networks",
      "created": "2025-02-14T16:05:11.271Z",
      "module": "inventory",
      "reason": "Network interface en0 updated",
      "type": [
        "change"
      ]
    },
    "host": {
      "ip": [
        "10.211.55.103"
      ],
      "mac": "00:1c:42:ce:16:50",
      "network": {
        "egress": {
          "bytes": 787456,
          "drops": 0,
          "errors": 0,
          "packets": 5797,
          "previous": {
            "bytes": 496640,
            "packets": 4869
          }
        },
        "ingress": {
          "bytes": 6020096,
          "drops": 0,
          "errors": 0,
          "packets": 7410,
          "previous": {
            "bytes": 5767168,
            "packets": 6478
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
        "10.211.55.255"
      ],
      "dhcp": null,
      "gateway": [
        "10.211.55.1"
      ],
      "metric": null,
      "netmask": [
        "255.255.255.0"
      ],
      "type": "ipv4"
    },
    "observer": {
      "ingress": {
        "interface": {
          "alias": null,
          "name": "en0"
        }
      }
    }
  },
  "fields": {
    "event.created": [
      "2025-02-14T16:05:11.271Z"
    ],
    "@timestamp": [
      "2025-02-14T16:05:21.000Z"
    ]
  },
  "sort": [
    1739549121000
  ]
}
```

## Ports

### Linux (Ubuntu 22.04)

<a name="Ports-Linux-Stateful"></a>Stateful

```json
{
  "_index": "wazuh-states-inventory-ports",
  "_id": "c9254e4ac1a542e9785807e7d9a714ba96da8ded",
  "_version": 1,
  "_score": 0,
  "_source": {
    "agent": {
      "id": "87676c24-26ea-4505-8618-e049b901f389",
      "name": "vm-ubuntu2204-agent",
      "groups": [],
      "type": "Endpoint",
      "version": "5.0.0",
      "host": {
        "architecture": "x86_64",
        "hostname": "vm-ubuntu2204-agent",
        "ip": [
          "10.0.2.15",
          "fe80::a00:27ff:fefc:450",
          "192.168.56.4",
          "fe80::a00:27ff:fe04:2d5b"
        ],
        "os": {
          "name": "Ubuntu",
          "type": "Linux",
          "version": "22.04.2 LTS (Jammy Jellyfish)"
        }
      }
    },
    "@timestamp": "2025-02-13T18:13:33.646Z",
    "destination": {
      "ip": [
        "0.0.0.0"
      ],
      "port": 0
    },
    "file": {
      "inode": 21315
    },
    "host": {
      "network": {
        "egress": {
          "queue": 0
        },
        "ingress": {
          "queue": 0
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
      "name": "sshd",
      "pid": 1214
    },
    "source": {
      "ip": [
        "0.0.0.0"
      ],
      "port": 22
    }
  },
  "fields": {
    "@timestamp": [
      "2025-02-13T18:13:33.646Z"
    ]
  }
}
```


<a name="Ports-Linux-Stateless"></a>Stateless

```json
{
  "_index": "wazuh-alerts-5.x-0001",
  "_id": "MT_OAJUBUF68aH7ENcu_",
  "_version": 1,
  "_score": 0,
  "_source": {
    "@timestamp": "2025-02-13T19:34:28Z",
    "agent": {
      "groups": [],
      "host": {
        "architecture": "x86_64",
        "hostname": "vm-ubuntu2204-agent",
        "ip": [
          "10.0.2.15",
          "fe80::a00:27ff:fefc:450",
          "192.168.56.4",
          "fe80::a00:27ff:fe04:2d5b"
        ],
        "os": {
          "name": "Ubuntu",
          "type": "Linux",
          "version": "22.04.2 LTS (Jammy Jellyfish)"
        }
      },
      "id": "87676c24-26ea-4505-8618-e049b901f389",
      "name": "vm-ubuntu2204-agent",
      "type": "Endpoint",
      "version": "5.0.0"
    },
    "destination": {
      "ip": [
        "192.168.56.131"
      ],
      "port": 27000
    },
    "event": {
      "action": "port-detected",
      "category": [
        "network"
      ],
      "collector": "ports",
      "created": "2025-02-13T19:34:18.504Z",
      "module": "inventory",
      "reason": "New connection from source port 45750 to destination port 27000",
      "type": [
        "connection"
      ]
    },
    "file": {
      "inode": 0
    },
    "host": {
      "network": {
        "egress": {
          "queue": 0
        },
        "ingress": {
          "queue": 0
        }
      }
    },
    "interface": {
      "state": "time_wait"
    },
    "network": {
      "protocol": "tcp"
    },
    "process": {
      "name": null,
      "pid": null
    },
    "source": {
      "ip": [
        "192.168.56.4"
      ],
      "port": 45750
    }
  },
  "fields": {
    "event.created": [
      "2025-02-13T19:34:18.504Z"
    ],
    "@timestamp": [
      "2025-02-13T19:34:28.000Z"
    ]
  },
  "highlight": {
    "event.action": [
      "@opensearch-dashboards-highlighted-field@port-detected@/opensearch-dashboards-highlighted-field@"
    ]
  },
  "sort": [
    0,
    null
  ]
}
```


### Windows (Windows 10)

<a name="Ports-Windows-Stateful"></a>Stateful

```json
{
  "_index": "wazuh-states-inventory-ports",
  "_id": "cd7c1d22cf7f7e702a7b1a550d7337bad0960183",
  "_version": 1,
  "_score": 0,
  "_source": {
    "agent": {
      "id": "51728df7-57d2-47b6-a065-a22995087bd7",
      "name": "DESKTOP-U8OHD3A",
      "groups": [],
      "type": "Endpoint",
      "version": "5.0.0",
      "host": {
        "architecture": "x86_64",
        "hostname": "DESKTOP-U8OHD3A",
        "ip": [
          "192.168.124.1",
          "fe80::4a97:2e71:39f4:21e5"
        ],
        "os": {
          "name": "Microsoft Windows 10 Home",
          "type": "Unknown",
          "version": "10.0.19045.5371"
        }
      }
    },
    "@timestamp": "2025-02-14T23:09:42.160Z",
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
      "name": "System",
      "pid": 4
    },
    "source": {
      "ip": [
        "0.0.0.0"
      ],
      "port": 5357
    }
  },
  "fields": {
    "@timestamp": [
      "2025-02-14T23:09:42.160Z"
    ]
  }
}
```

<a name="Ports-Windows-Stateless"></a>Stateless

```json
{
  "_index": "wazuh-alerts-5.x-0001",
  "_id": "W6X_FZUBJYGzKyMzhXVS",
  "_version": 1,
  "_score": null,
  "_source": {
    "@timestamp": "2025-02-17T22:20:18Z",
    "agent": {
      "groups": [],
      "host": {
        "architecture": "x86_64",
        "hostname": "DESKTOP-U8OHD3A",
        "ip": [
          "192.168.124.1",
          "fe80::4a97:2e71:39f4:21e5"
        ],
        "os": {
          "name": "Microsoft Windows 10 Home",
          "type": "Unknown",
          "version": "10.0.19045.5487"
        }
      },
      "id": "4252f628-46c1-43c4-a184-f19887e768fc",
      "name": "DESKTOP-U8OHD3A",
      "type": "Endpoint",
      "version": "5.0.0"
    },
    "destination": {
      "ip": [],
      "port": 0
    },
    "event": {
      "action": "port-updated",
      "category": [
        "network"
      ],
      "changed_fields": [
        "process.pid"
      ],
      "collector": "ports",
      "created": "2025-02-17T22:20:14.210Z",
      "module": "inventory",
      "reason": "Updated connection from source port 5353 to destination port 0",
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
      "state": null
    },
    "network": {
      "protocol": "udp"
    },
    "process": {
      "name": "chrome.exe",
      "pid": 14432,
      "previous": {
        "pid": 3252
      }
    },
    "source": {
      "ip": [
        "0.0.0.0"
      ],
      "port": 5353
    }
  },
  "fields": {
    "event.created": [
      "2025-02-17T22:20:14.210Z"
    ],
    "@timestamp": [
      "2025-02-17T22:20:18.000Z"
    ]
  },
  "highlight": {
    "event.action": [
      "@opensearch-dashboards-highlighted-field@port-updated@/opensearch-dashboards-highlighted-field@"
    ]
  },
  "sort": [
    1739830818000
  ]
}
```

### Macos (macOS Sonoma 14.4.1)

<a name="Ports-Macos-Stateful"></a>Stateful

```json
{
  "_index": "wazuh-states-inventory-ports",
  "_id": "d4f3fd3ae458959c772fe50defabc33ce243635c",
  "_version": 1,
  "_score": 0,
  "_source": {
    "agent": {
      "id": "e7aadfe0-1971-4351-9767-34a330be3c4e",
      "name": "idr-2097-sonoma-14-9903",
      "groups": [],
      "type": "Endpoint",
      "version": "5.0.0",
      "host": {
        "architecture": "x86_64",
        "hostname": "idr-2097-sonoma-14-9903",
        "ip": [
          "10.211.55.103",
          "fe80::5c:92d1:bd06:96e5"
        ],
        "os": {
          "name": "macOS",
          "type": "Darwin",
          "version": "14.4.1"
        }
      }
    },
    "@timestamp": "2025-02-14T15:38:37.835Z",
    "destination": {
      "ip": [
        "54.211.138.25"
      ],
      "port": 27000
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
      "state": "established"
    },
    "network": {
      "protocol": "tcp"
    },
    "process": {
      "name": "wazuh-agent",
      "pid": 1898
    },
    "source": {
      "ip": [
        "10.211.55.103"
      ],
      "port": 49157
    }
  },
  "fields": {
    "@timestamp": [
      "2025-02-14T15:38:37.835Z"
    ]
  }
}
```

<a name="Ports-Macos-Stateless"></a>Stateless

```json
{
  "_index": "wazuh-alerts-5.x-0001",
  "_id": "i1A1BZUBA1q-qsf1H5oG",
  "_version": 1,
  "_score": null,
  "_source": {
    "@timestamp": "2025-02-14T16:05:21Z",
    "agent": {
      "groups": [],
      "host": {
        "architecture": "x86_64",
        "hostname": "idr-2097-sonoma-14-9903",
        "ip": [
          "10.211.55.103",
          "fe80::5c:92d1:bd06:96e5"
        ],
        "os": {
          "name": "macOS",
          "type": "Darwin",
          "version": "14.4.1"
        }
      },
      "id": "e7aadfe0-1971-4351-9767-34a330be3c4e",
      "name": "idr-2097-sonoma-14-9903",
      "type": "Endpoint",
      "version": "5.0.0"
    },
    "destination": {
      "ip": [
        "54.211.138.25"
      ],
      "port": 27000
    },
    "event": {
      "action": "port-detected",
      "category": [
        "network"
      ],
      "collector": "ports",
      "created": "2025-02-14T16:05:11.271Z",
      "module": "inventory",
      "reason": "New connection from source port 49210 to destination port 27000",
      "type": [
        "connection"
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
      "state": "established"
    },
    "network": {
      "protocol": "tcp"
    },
    "process": {
      "name": "wazuh-agent",
      "pid": 2249
    },
    "source": {
      "ip": [
        "10.211.55.103"
      ],
      "port": 49210
    }
  },
  "fields": {
    "event.created": [
      "2025-02-14T16:05:11.271Z"
    ],
    "@timestamp": [
      "2025-02-14T16:05:21.000Z"
    ]
  },
  "sort": [
    1739549121000
  ]
}
```

## Hotfixes
* taken directly from the server (not the indexer)

### Windows (Windows 10)

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
