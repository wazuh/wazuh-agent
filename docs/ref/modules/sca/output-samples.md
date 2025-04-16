## Stateful

- create

```json
{
    "agent":
    {
        "groups":
        [
            "validYaml"
        ],
        "host":
        {
            "architecture": "x86_64",
            "hostname": "asus",
            "ip":
            [
                "172.17.0.1",
                "fe80::42:c6ff:fe72:8839",
                "fe80::a8fd:dbff:fe87:56f8",
                "192.168.1.32",
                "fd5d:5535:8c7f:93b2:2fe7:a53c:f780:f22d"
            ],
            "os":
            {
                "name": "Ubuntu",
                "type": "Linux",
                "version": "22.04.5 LTS (Jammy Jellyfish)"
            }
        },
        "id": "d9a65288-11bc-4362-8198-fa4939116a73",
        "name": "dummy",
        "type": "Endpoint",
        "version": "v5.0.0"
    }
}
{
    "id": "71c10b3cabf0f88919aa15fdaa8fed1719c9fe1f",
    "module": "sca",
    "operation": "create"
}
{
    "check":
    {
        "compliance":
        [
            "int:2.1.1"
        ],
        "condition": "all",
        "description": "Disabling SMBv1 mitigates known vulnerabilities.",
        "id": "CUST001",
        "name": "Ensure SMBv1 is disabled.",
        "rationale": "SMBv1 is outdated and insecure.",
        "reason": null,
        "references":
        [
            "https://internal.docs/policies/windows"
        ],
        "remediation": "Set 'SMB1' registry key to 0.",
        "result": "Not run",
        "rules":
        [
            "r:HKLM\\SYSTEM\\CurrentControlSet\\Services\\LanmanServer\\Parameters -> n:SMB1 compare == 0"
        ]
    },
    "policy":
    {
        "description": "Custom internal hardening guidelines for Windows",
        "file": "custom_windows_policy.yml",
        "id": "custom_policy_win",
        "name": "Custom Windows Hardening Policy",
        "references":
        [
            "https://internal.docs/policies/windows"
        ]
    },
    "timestamp": "2025-04-16T12:10:01.481Z"
}

```

- update

```json
{
    "agent":
    {
        "groups":
        [
            "validYaml"
        ],
        "host":
        {
            "architecture": "x86_64",
            "hostname": "asus",
            "ip":
            [
                "172.17.0.1",
                "fe80::42:c6ff:fe72:8839",
                "fe80::a8fd:dbff:fe87:56f8",
                "192.168.1.32",
                "fd5d:5535:8c7f:93b2:2fe7:a53c:f780:f22d"
            ],
            "os":
            {
                "name": "Ubuntu",
                "type": "Linux",
                "version": "22.04.5 LTS (Jammy Jellyfish)"
            }
        },
        "id": "d9a65288-11bc-4362-8198-fa4939116a73",
        "name": "dummy",
        "type": "Endpoint",
        "version": "v5.0.0"
    }
}
{
    "id": "71c10b3cabf0f88919aa15fdaa8fed1719c9fe1f",
    "module": "sca",
    "operation": "update"
}
{
    "check":
    {
        "compliance":
        [
            "int:2.1.1"
        ],
        "condition": "all",
        "description": "Disabling SMBv1 mitigates known vulnerabilities.",
        "id": "CUST001",
        "name": "Ensure SMBv1 is disabled.",
        "rationale": "SMBv1 is outdated and insecure.",
        "reason": null,
        "references":
        [
            "https://internal.docs/policies/windows"
        ],
        "remediation": "Set 'SMB1' registry key to 0.",
        "result": "failed",
        "rules":
        [
            "r:HKLM\\SYSTEM\\CurrentControlSet\\Services\\LanmanServer\\Parameters -> n:SMB1 compare == 0"
        ]
    },
    "policy":
    {
        "description": "Custom internal hardening guidelines for Windows systems",
        "file": "custom_windows_policy.yml",
        "id": "custom_policy_win",
        "name": "Custom Windows Hardening Policy",
        "references":
        [
            "https://internal.docs/policies/windows"
        ]
    },
    "timestamp": "2025-04-16T12:14:55.026Z"
}
```

- delete

```json
{
    "agent":
    {
        "groups":
        [
            "validYaml"
        ],
        "host":
        {
            "architecture": "x86_64",
            "hostname": "asus",
            "ip":
            [
                "172.17.0.1",
                "fe80::42:c6ff:fe72:8839",
                "fe80::a8fd:dbff:fe87:56f8",
                "192.168.1.32",
                "fd5d:5535:8c7f:93b2:2fe7:a53c:f780:f22d"
            ],
            "os":
            {
                "name": "Ubuntu",
                "type": "Linux",
                "version": "22.04.5 LTS (Jammy Jellyfish)"
            }
        },
        "id": "d9a65288-11bc-4362-8198-fa4939116a73",
        "name": "dummy",
        "type": "Endpoint",
        "version": "v5.0.0"
    }
}
{
    "id": "72a1c51db19f3f52d6ee57b94e05919e885e4bad",
    "module": "sca",
    "operation": "delete"
}
```

## Stateless

- creation

```json
{
    "agent":
    {
        "groups":
        [
            "validYaml"
        ],
        "host":
        {
            "architecture": "x86_64",
            "hostname": "asus",
            "ip":
            [
                "172.17.0.1",
                "fe80::42:c6ff:fe72:8839",
                "fe80::a8fd:dbff:fe87:56f8",
                "192.168.1.32",
                "fd5d:5535:8c7f:93b2:2fe7:a53c:f780:f22d"
            ],
            "os":
            {
                "name": "Ubuntu",
                "type": "Linux",
                "version": "22.04.5 LTS (Jammy Jellyfish)"
            }
        },
        "id": "d9a65288-11bc-4362-8198-fa4939116a73",
        "name": "dummy",
        "type": "Endpoint",
        "version": "v5.0.0"
    }
}
{
    "collector": "check",
    "module": "sca"
}
{
    "check":
    {
        "compliance":
        [
            "int:2.1.1"
        ],
        "condition": "all",
        "description": "Disabling SMBv1 mitigates known vulnerabilities.",
        "id": "CUST001",
        "name": "Ensure SMBv1 is disabled.",
        "rationale": "SMBv1 is outdated and insecure.",
        "reason": null,
        "references":
        [
            "https://internal.docs/policies/windows"
        ],
        "remediation": "Set 'SMB1' registry key to 0.",
        "result": "Not run",
        "rules":
        [
            "r:HKLM\\SYSTEM\\CurrentControlSet\\Services\\LanmanServer\\Parameters -> n:SMB1 compare == 0"
        ]
    },
    "event":
    {
        "action":
        [
            "creation-check"
        ],
        "category":
        [
            "configuration"
        ],
        "changed_fields":
        [],
        "created": "2025-04-16T12:10:01.486Z",
        "type": "creation"
    },
    "policy":
    {
        "description": "Custom internal hardening guidelines for Windows",
        "file": "custom_windows_policy.yml",
        "id": "custom_policy_win",
        "name": "Custom Windows Hardening Policy",
        "references":
        [
            "https://internal.docs/policies/windows"
        ]
    }
}
```

- change

When only a policy-related field changes, the `collector` field is set to `policy`, and the modified fields will appear
under the `previous` key within the policy object. Additionally, the modified fields will be listed in
`event.changed_fields`.

```json
{
    "agent":
    {
        "groups":
        [
            "validYaml"
        ],
        "host":
        {
            "architecture": "x86_64",
            "hostname": "asus",
            "ip":
            [
                "172.17.0.1",
                "fe80::42:c6ff:fe72:8839",
                "fe80::a8fd:dbff:fe87:56f8",
                "192.168.1.32",
                "fd5d:5535:8c7f:93b2:2fe7:a53c:f780:f22d"
            ],
            "os":
            {
                "name": "Ubuntu",
                "type": "Linux",
                "version": "22.04.5 LTS (Jammy Jellyfish)"
            }
        },
        "id": "d9a65288-11bc-4362-8198-fa4939116a73",
        "name": "dummy",
        "type": "Endpoint",
        "version": "v5.0.0"
    }
}
{
    "collector": "policy",
    "module": "sca"
}
{
    "check":
    {
        "compliance":
        [
            "int:2.1.1"
        ],
        "condition": "all",
        "description": "Disabling SMBv1 mitigates known vulnerabilities.",
        "id": "CUST001",
        "name": "Ensure SMBv1 is disabled.",
        "rationale": "SMBv1 is outdated and insecure.",
        "reason": null,
        "references":
        [
            "https://internal.docs/policies/windows"
        ],
        "remediation": "Set 'SMB1' registry key to 0.",
        "result": "failed",
        "rules":
        [
            "r:HKLM\\SYSTEM\\CurrentControlSet\\Services\\LanmanServer\\Parameters -> n:SMB1 compare == 0"
        ]
    },
    "event":
    {
        "action":
        [
            "policy-change"
        ],
        "category":
        [
            "configuration"
        ],
        "changed_fields":
        [
            "policy.description"
        ],
        "created": "2025-04-16T12:14:55.029Z",
        "type": "change"
    },
    "policy":
    {
        "description": "Custom internal hardening guidelines for Windows systems",
        "file": "custom_windows_policy.yml",
        "id": "custom_policy_win",
        "name": "Custom Windows Hardening Policy",
        "previous":
        {
            "description": "Custom internal hardening guidelines for Windows"
        },
        "references":
        [
            "https://internal.docs/policies/windows"
        ]
    }
}
```

When a check-related field changes (possibly along with a policy change), the `collector` field is set to `check`, and
the modified fields will appear under the `previous` key within the check object (and within the policy object if
applicable). As in the previous case, the modified fields will be listed in `changed_fields`.

```json
  {
    "agent":
    {
        "groups":
        [
            "validYaml"
        ],
        "host":
        {
            "architecture": "x86_64",
            "hostname": "asus",
            "ip":
            [
                "172.17.0.1",
                "fe80::42:c6ff:fe72:8839",
                "fe80::a8fd:dbff:fe87:56f8",
                "192.168.1.32",
                "fd5d:5535:8c7f:93b2:2fe7:a53c:f780:f22d"
            ],
            "os":
            {
                "name": "Ubuntu",
                "type": "Linux",
                "version": "22.04.5 LTS (Jammy Jellyfish)"
            }
        },
        "id": "d9a65288-11bc-4362-8198-fa4939116a73",
        "name": "dummy",
        "type": "Endpoint",
        "version": "v5.0.0"
    }
}
{
    "collector": "check",
    "module": "sca"
}
{
    "check":
    {
        "compliance":
        [
            "cis:1.2.1",
            "cis_csc:16.11"
        ],
        "condition": "any",
        "description": "Limits the number of failed logon attempts.",
        "id": "26001",
        "name": "Ensure 'Account lockout threshold' is set to 5 or fewer invalid logon attempts.",
        "previous":
        {
            "rules": "c:secedit.exe /export -> n:LockoutThreshold:\\s+(\\d+) compare <= 4"
        },
        "rationale": "Prevents brute-force logon attempts.",
        "reason": null,
        "references":
        [
            "https://workbench.cisecurity.org"
        ],
        "remediation": "Set the value in Local Security Policy > Account Lockout Policy",
        "result": "Not run",
        "rules":
        [
            "c:secedit.exe /export -> n:LockoutThreshold:\\s+(\\d+) compare <= 5"
        ]
    },
    "event":
    {
        "action":
        [
            "check-change"
        ],
        "category":
        [
            "configuration"
        ],
        "changed_fields":
        [
            "check.rules",
            "policy.file"
        ],
        "created": "2025-04-16T12:14:55.046Z",
        "type": "change"
    },
    "policy":
    {
        "description": "This document provides prescriptive guidance for Windows 11 Enterprise",
        "file": "cis_win11_enterprise1.yml",
        "id": "cis_win11_enterprise_21H2",
        "name": "CIS Microsoft Windows 11 Enterprise Benchmark v1.0.0",
        "previous":
        {
            "file": "cis_win11_enterprise.yml"
        },
        "references":
        [
            "https://www.cisecurity.org/cis-benchmarks/"
        ]
    }
}
```

- deletion

```json
{
    "agent":
    {
        "groups":
        [
            "validYaml"
        ],
        "host":
        {
            "architecture": "x86_64",
            "hostname": "asus",
            "ip":
            [
                "172.17.0.1",
                "fe80::42:c6ff:fe72:8839",
                "fe80::a8fd:dbff:fe87:56f8",
                "192.168.1.32",
                "fd5d:5535:8c7f:93b2:2fe7:a53c:f780:f22d"
            ],
            "os":
            {
                "name": "Ubuntu",
                "type": "Linux",
                "version": "22.04.5 LTS (Jammy Jellyfish)"
            }
        },
        "id": "d9a65288-11bc-4362-8198-fa4939116a73",
        "name": "dummy",
        "type": "Endpoint",
        "version": "v5.0.0"
    }
}
{
    "collector": "check",
    "module": "sca"
}
{
    "check":
    {
        "compliance":
        [
            "cis:1.1.1",
            "cis_csc:5.2"
        ],
        "condition": "all",
        "description": "This policy setting determines how many previous passwords are remembered.",
        "id": "26000",
        "name": "Ensure 'Enforce password history' is set to '24 or more password(s)'.",
        "rationale": "The longer a user uses the same password ...",
        "reason": null,
        "references":
        [
            "https://workbench.cisecurity.org"
        ],
        "remediation": "To establish the recommended configuration ...",
        "result": "failed",
        "rules":
        [
            "c:net.exe accounts -> n:Maximum password age \\(days\\):\\s+(\\d+) compare > 0",
            "c:net.exe accounts -> n:Length of password history maintained:\\s+(\\d+) compare >= 24"
        ]
    },
    "event":
    {
        "action":
        [
            "check-deletion"
        ],
        "category":
        [
            "configuration"
        ],
        "changed_fields":
        [],
        "created": "2025-04-16T12:14:55.033Z",
        "type": "deletion"
    },
    "policy":
    {
        "description": "This document provides prescriptive guidance for Windows 11 Enterprise",
        "file": "cis_win11_enterprise1.yml",
        "id": "cis_win11_enterprise_21H2",
        "name": "CIS Microsoft Windows 11 Enterprise Benchmark v1.0.0",
        "references":
        [
            "https://www.cisecurity.org/cis-benchmarks/"
        ]
    }
}
```
