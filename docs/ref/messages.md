# Messages

## Enrollment requests

- **API**: Server Management

### Authentication

- **Endpoint**: `/security/user/authenticate`

- **Method**: POST

- **Headers**:
  - **Authorization**: Basic `base64(user:pass)`
  - **User-Agent**:
    - Product name
    - Agent's version
    - Agent's type
    - Architecture
    - OS type

Example: `WazuhXDR/x.y.z (Endpoint; x86_64; Linux)`

- **Body**: _empty_

### Enrollment

- **Endpoint**: `/agents`

- **Method**: POST

- **Headers**:
  - **Authorization**: Bearer `token`
  - **User-Agent**:
    - Product name
    - Agent's version
    - Agent's type
    - Architecture
    - OS type

Example: `WazuhXDR/x.y.z (Endpoint; x86_64; Linux)`

- **Body**: _json_
  - **id**: Agent's UUID
  - **name**: Agent's name
  - **key**: Agent's alphanumeric key (32 characters)
  - **type**: Agent's type
  - **version**: Agent's version
  - **host**:
    - **os**:
      - **name**: OS name
      - **type**: OS type
      - **version**: OS version
    - **architecture**: Architecture
    - **hostname**: Hostname
    - **ip**: List of IPs

Example:

```json
{
    "id": "0194857f-39b5-7bfb-a7bc-c5c6835f03d1",
    "name": "agent_name",
    "key": "eJdcN3mtVcLuyNwLj1Kx0NCvH53R16no",
    "type": "Endpoint",
    "version": "x.y.z",
    "host": {
        "architecture": "aarch64",
        "hostname": "desktop",
        "ip": ["172.20.0.1"],
        "os": {
            "name": "Ubuntu",
            "type": "Linux",
            "version": "24.04"
        }
    }
}
```

> **_NOTE:_** An agent will always enroll as new regardless of whether it has previously enrolled.

## Communication requests

- **API**: Agent Comms

### Authentication

- **Endpoint**: `/api/v1/authentication`

- **Method**: POST

- **Headers**:
  - **User-Agent**:
    - Product name
    - Agent's version
    - Agent's type
    - Architecture
    - OS type

Example: `WazuhXDR/x.y.z (Endpoint; x86_64; Linux)`

- **Body**: _json_
  - **uuid**: Agent's UUID
  - **key**: Agent's alphanumeric key (32 characters)

Example:

```json
{
    "uuid": "0194857f-39b5-7bfb-a7bc-c5c6835f03d1",
    "key": "eJdcN3mtVcLuyNwLj1Kx0NCvH53R16no"
}
```

### Sateful/stateless

- **Endpoint**: `/api/v1/events/stateful` & `/api/v1/events/stateless`

- **Method**: POST

- **Headers**:
  - **Authorization**: Bearer `token`
  - **User-Agent**:
    - Product name
    - Agent's version
    - Agent's type
    - Architecture
    - OS type

Example: `WazuhXDR/x.y.z (Endpoint; x86_64; Linux)`

- **Body**: _json_
  - **id**: Agent's UUID
  - **name**: Agent's name
  - **type**: Agent's type
  - **version**: Agent's version
  - **groups**: Agent's groups
  - **host**:
    - **os**:
      - **name**: OS name
      - **type**: OS type
      - **version**: OS version
    - **architecture**: Architecture
    - **hostname**: Hostname
    - **ip**: List of IPs

Example:

```json
{
    "agent": {
        "id": "0194857f-39b5-7bfb-a7bc-c5c6835f03d1",
        "name": "agent_name",
        "type": "Endpoint",
        "version": "x.y.z",
        "groups": [],
        "host": {
            "architecture": "aarch64",
            "hostname": "desktop",
            "ip": ["172.20.0.1"],
            "os": {
                "name": "Ubuntu",
                "type": "Linux",
                "version": "24.04"
            }
        }
    }
}
```

### Commands

- **Endpoint**: `/api/v1/commands`

- **Method**: GET

- **Headers**:
  - **Authorization**: Bearer `token`
  - **User-Agent**:
    - Product name
    - Agent's version
    - Agent's type
    - Architecture
    - OS type

Example: `WazuhXDR/x.y.z (Endpoint; x86_64; Linux)`

- **Body**: _empty_
