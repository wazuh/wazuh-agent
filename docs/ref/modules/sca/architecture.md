# Architecture

## Tables

### Policy Table

```sql
CREATE TABLE IF NOT EXISTS sca_policy (
    id TEXT PRIMARY KEY,
    name TEXT,
    file TEXT,
    description TEXT,
    refs TEXT
);
```

This table stores data for each security policy used in the SCA module, including its unique ID, name, source file,
description, and external references.

| Mandatory | Column        | Data Type | Description                                           | ECS Mapping | ECS Data Type |
| :-------: | ------------- | --------- | ----------------------------------------------------- | ----------- | ------------- |
|     ✔️    | `id`          | TEXT      | Unique identifier of the policy.                      | N/A         | keyword       |
|           | `name`        | TEXT      | Human-readable name of the policy.                    | N/A         | keyword       |
|           | `file`        | TEXT      | Path to the policy definition file.                   | N/A         | keyword       |
|           | `description` | TEXT      | Description of the policy purpose and content.        | N/A         | keyword       |
|           | `refs`        | TEXT      | External references related to the policy (e.g. CIS). | N/A         | keyword       |

---

### Check Table

```sql
CREATE TABLE IF NOT EXISTS sca_check (
    id TEXT PRIMARY KEY,
    policy_id TEXT REFERENCES sca_policy(id),
    name TEXT,
    description TEXT,
    rationale TEXT,
    remediation TEXT,
    refs TEXT,
    result TEXT DEFAULT 'Not run',
    reason TEXT,
    condition TEXT,
    compliance TEXT,
    rules TEXT
);
```

This table stores individual checks associated with a policy. Each check includes metadata, logic conditions, rules to
be evaluated, and tracking fields for results and compliance.

| Mandatory | Column        | Data Type | Description                                                               | ECS Mapping | ECS Data Type |
| :-------: | ------------- | --------- | ------------------------------------------------------------------------- | ----------- | ------------- |
|     ✔️    | `id`          | TEXT      | Unique identifier of the check.                                           | N/A         | keyword       |
|     ✔️    | `policy_id`   | TEXT      | Reference to the associated policy ID.                                    | N/A         | keyword       |
|           | `name`        | TEXT      | Short name summarizing the check.                                        | N/A         | keyword       |
|           | `description` | TEXT      | Detailed explanation of what the check evaluates.                         | N/A         | keyword       |
|           | `rationale`   | TEXT      | Justification or reason behind the check.                                 | N/A         | keyword       |
|           | `remediation` | TEXT      | Instructions for correcting a failed check.                               | N/A         | keyword       |
|           | `refs`        | TEXT      | External references related to the check.                                 | N/A         | keyword       |
|           | `result`      | TEXT      | Current evaluation result (e.g. Passed, Failed, Not run, Not applicable). | N/A         | keyword       |
|           | `reason`      | TEXT      | Explanation for the check's result.                                       | N/A         | keyword       |
|           | `condition`   | TEXT      | Logical condition under which the check applies.                          | N/A         | keyword       |
|           | `compliance`  | TEXT      | Compliance mapping (e.g., CIS ID, NIST tag).                              | N/A         | keyword       |
|           | `rules`       | TEXT      | Serialized rule(s) logic used to perform the actual check.                | N/A         | keyword       |