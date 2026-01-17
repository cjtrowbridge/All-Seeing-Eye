# Playbook: Using the API to Interact with Nodes

*Status: Active*

## Objective
Provide a consistent, safe procedure for agents and users to interact with nodes via the Python API and validate firmware versions and endpoint availability without introducing configuration drift.

## Preconditions
- Python environment with dependencies installed from `api/requirements.txt`.
- Access to the local network hosting the nodes.

## 1. Validate Firmware Version (Build Hash)

**Goal**: Confirm that a node is running the same firmware build as the repository.

1.  **Read the expected build hash from the repo**:
    *   File: `firmware/AllSeeingEye/src/BuildVersion.h`
    *   Field: `BUILD_ID`

2.  **Fetch the node’s reported build hash**:
    *   Endpoint: `GET /api/status`
    *   Field: `build_id`

3.  **Compare**:
    *   If `status.build_id != BUILD_ID`, the node is running a different firmware build.
    *   Do not assume missing fields are a bug until versions are aligned.

**Python Example**:
```python
from client import EyeClient

client = EyeClient("192.168.1.111")
status = client.get("/api/status")
print(status["data"]["build_id"])
```

## 2. Discover and Select a Target Node

**Rule**: Always target nodes by **hostname**, or **description** (quoted if spaces). (IP Addresses change frequently, and future versions will support connections across meshtastic where ip addresses don't exist.)

1.  **List known hosts** (auto-discovers if none exist):
    ```bash
    python api/cli.py --list
    ```

2.  **Target by hostname**:
    ```bash
    python api/cli.py --host "allseeingeye-xyzabc" get /api/status
    ```

3.  **Target by description**:
    ```bash
    python api/cli.py --host "garage" get /api/status
    ```

## 3. Endpoint Inventory

**Authoritative Reference**:  
The complete list of API endpoints, methods, and payloads is maintained in **[`api/README.md`](../api/README.md)**. Please refer to that document for the full protocol specification.

The examples below demonstrate the *syntax* for interacting with these endpoints using the Python client.

**CLI Parity Rule**: Every endpoint in `api/README.md` must be callable from the CLI for debugging and verification. If a dedicated command does not exist, use the generic `get`/`post` form and document the example.

## 4. Python API Usage Patterns

**General GET**:
```python
from client import EyeClient

client = EyeClient("192.168.1.111")
status = client.get("/api/status")
print(status)
```

**POST with payload**:
```python
from client import EyeClient

client = EyeClient("192.168.1.111")
response = client.post("/api/config", {
    "description": "lab node",
    "timezone": "America/Los_Angeles"
})
print(response)
```

**Cluster Task Deploy**:
```bash
python api/cli.py post /api/cluster/deploy '{"task":"spectrum/scan", "params": {"start": 902.0, "stop": 928.0}}'
```

**Cluster Task Start**:
```bash
python api/cli.py post /api/cluster/start '{}'
```

**Cluster Report**:
```bash
python api/cli.py get /api/report
```

**LED Set (GET with query params)**:
```python
from client import EyeClient

client = EyeClient("192.168.1.111")
response = client.get("/api/led?r=255&g=0&b=0")
print(response)
```

**Ping a peer**:
```python
from client import EyeClient

client = EyeClient("192.168.1.111")
response = client.get("/api/ping?target=192.168.1.106")
print(response)
```

## 5. Safety and Consistency Checks

- **Version Mismatch**: Treat missing fields as firmware version mismatch first.
- **No TX/Passive Only**: Do not add any RF transmit logic; firmware is passive-only by policy.
- **Always Log**: When changing config or rebooting, capture logs (`/api/logs`) before and after.

## 6. Verification Checklist

1.  `GET /api/status` returns `build_id` and expected fields.
2.  `GET /api/config` includes required fields (e.g., `timezone`).
3.  `GET /api/peers` returns expected peers for the node.
4.  `POST /api/config` returns success and changes appear in subsequent GET.
5.  `GET /api/logs/head` contains startup events if troubleshooting boot issues.
