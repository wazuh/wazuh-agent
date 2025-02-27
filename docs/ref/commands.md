# Commands

This section describes the defined commands that the agent can execute and the structure they must follow.

## Centralized Configuration

### **`set-group`**

This centralized configuration command is responsible for assigning the groups to which the agent belongs. Each command received must include the complete list of groups in its parameters, including any previously assigned groups, if any, as well as the new groups. If this list of groups is empty, all previously assigned groups will be removed.

The execution of this command triggers the download of the configuration files for each group and subsequently initiates a reload of the configuration and modules to apply the new settings.

```json
{
    "action":
    {
        "args":
        {
            "groups":
            [
                "group_1",
                "group_2",
                "...",
                "group_N"
            ],
        },
        "name": "set-group",
        "version": "x.y.z"
    },
    "source": "Users/Services",
    "document_id": "A8-62pMBBmC6Jrvqj9kW",
    "user": "Management API",
    "target":
    {
        "id": "d5b250c4-dfa1-4d94-827f-9f99210dbe6c",
        "type": "agent"
    }
}
```

### **`fetch-config`**

This centralized configuration command is responsible for fetching the configuration of the groups to which the agent belongs. This command does not require any arguments.

Executing this command triggers a reload of the configuration and modules to ensure the new settings are applied.

```json
{
    "action":
    {
        "args":
        {},
        "name": "fetch-config",
        "version": "x.y.z"
    },
    "source": "Users/Services",
    "document_id": "A8-62pMBBmC6Jrvqj9kW",
    "user": "Management API",
    "target":
    {
        "id": "d5b250c4-dfa1-4d94-827f-9f99210dbe6c",
        "type": "agent"
    }
}
```

## Restart Handler

The restart handler is responsible for restarting the agent. This command does not require any arguments.

Executing this command triggers a restart, which can be processed in two ways:

- If the service was started by systemd, it will use the same command to restart the agent.
- If it was called manually, it will use the same command with the same arguments that were used to call it.

```json
{
    "action":
    {
        "args":
        {},
        "name": "restart",
        "version": "x.y.z"
    },
    "source": "Users/Services",
    "document_id": "A8-62pMBBmC6Jrvqj9kW",
    "user": "Management API",
    "target":
    {
        "id": "d5b250c4-dfa1-4d94-827f-9f99210dbe6c",
        "type": "agent"
    }
}
```
