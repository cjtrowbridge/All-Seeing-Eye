# All Seeing Eye - Desktop API Client & CLI

This directory contains the Python-based control plane for the All Seeing Eye cluster. It serves as both a library for automated agents and a command-line interface (CLI) for manual operation and debugging.

## Design Goals

1.  **Zero-Config Discovery**: The system discovers nodes automatically and maintains a registry without manual IP entry.
2.  **Code-First Control**: Every firmware capability is accessible via a Python method.
3.  **Observability**: Provides formatted, human-readable output for debugging node internals (Status, NTP, Peers).

## Architecture

### 1. Known Hosts Management
To ensure reliable command routing without constant re-scanning, the API client maintains a local registry of nodes.

*   **File**: `api/known_hosts.csv`
*   **Format**: `ip_address,hostname,description,last_seen_timestamp`
*   **Behavior**:
    *   **Auto-Creation**: If missing, the API triggers a discovery scan.
    *   **Peer Crawl**: When connecting to any node, the client fetches its Peer Registry (`/api/peers`) and adds unknown peers to the local CSV.
    *   **Ordering**: The top row is considered the **Default Target** for CLI commands if no host is specified.

### 2. Peer Discovery
The discovery engine leverages multiple methods to build the `known_hosts.csv`:
*   **mDNS/Bonjour**: Listens for `_http._tcp` broadcasts.
*   **Peer Gossip**: Queries `GET /api/status` from known nodes and ingests the `peers` array.
*   **IP Scan**: (Fallback) Scans local subnets for valid API responses.

## CLI Usage

The CLI (`cli.py`) is the primary entry point for developers.

**Syntax**:
```bash
python cli.py [TARGET] [ACTION] [PAYLOAD]
```

*   **List Hosts**:
    ```bash
    python cli.py --list
    # Output:
    # 192.168.1.105 | eye-alpha | Living Room (Default)
    # 192.168.1.106 | eye-beta  | Office
    ```

*   **Default Target (Top Row)**:
    ```bash
    python cli.py get /api/status
    ```

*   **Specific Target**:
    ```bash
    python cli.py --host "eye-beta" get /api/config
    ```

*   **Specific Target (by Description)**:
    ```bash
    python cli.py --host "Living Room" get /api/status
    ```

*   **POST Command**:
    ```bash
    python cli.py post /api/task '{"plugin":"SpectrumSweep", "params":{"freq": 915}}'
    ```

## API Endpoints Reference

The following routes are registered in `firmware/AllSeeingEye/src/WebServer.cpp` and are the authoritative interface for the device.

| Endpoint | Method | Purpose |
| --- | --- | --- |
| `/api` | GET | Self-documentation of available endpoints |
| `/api/status` | GET | System state, NTP sync, peers, logs |
| `/api/config` | GET/POST | Read or update persisted configuration |
| `/api/fs` | GET | List files in LittleFS |
| `/api/peers` | GET | Peer registry from the node |
| `/api/ping?target=IP` | GET | Ping a specific IP |
| `/api/logs` | GET | Runtime log buffer (tail), timestamps use device timezone |
| `/api/logs/head` | GET | Startup log buffer (head), includes applied timezone entry |
| `/api/led?r=R&g=G&b=B` | GET | Set LED color and return LED status |
| `/api/led/on` | POST | Enable LED output |
| `/api/led/off` | POST | Disable LED output |
| `/api/queue` | GET | Task scheduler state |
| `/api/reboot` | POST | Reboot the device |

## Development Roadmap

This roadmap outlines the steps to build the `v1.0` Python API.

### Phase 1: Foundation (Host Management)
- [ ] **Task 1.1**: Create `HostManager` class.
    - [ ] Implement `load_hosts()` to parse `known_hosts.csv`.
    - [ ] Implement `save_hosts()` to write CSV with headers.
    - [ ] Implement `add_host(ip, hostname, desc)` with deduplication.
- [ ] **Task 1.2**: Implement Discovery Logic.
    - [ ] Create `Discovery` module using `zeroconf` (mDNS) or subnet scan.
    - [ ] Implement `scan_network()` function to populate manager.
    - [ ] Add logic to auto-trigger scan if CSV is empty on load.
- [ ] **Task 1.3**: Peer Integration.
    - [ ] Implement `crawl_peers(seed_host)`: Connect to host, get list, add to CSV.

### Phase 2: Core Client
- [ ] **Task 2.1**: Update `EyeClient` class.
    - [ ] Constructor takes `ip` or `hostname`.
    - [ ] Implement generic `_request(method, endpoint, payload)` wrapper.
    - [ ] Add timeout handling and JSON parsing.

### Phase 3: CLI Implementation
- [ ] **Task 3.1**: CLI Skeleton.
    - [ ] Set up `argparse` for `cli.py`.
    - [ ] specific arguments: `--list`, `--host`, `action` (GET/POST), `endpoint`.
- [ ] **Task 3.2**: Target Resolution.
    - [ ] Logic to select host: Specified IP/Name -> Map to CSV -> or Default (Row 0).
- [ ] **Task 3.3**: Action Handlers.
    - [ ] `handle_list()`: Print formatted table of known hosts.
    - [ ] `handle_request()`: Execute generic GET/POST against target.
    - [ ] Pretty-print JSON responses to terminal.

### Phase 4: Debugging & Verification
- [ ] **Task 4.1**: NTP Debugging.
    - [ ] Run `cli.py get /api/status` against fleet.
    - [ ] Verify `time` and `ntp_sync` fields are populated correctly.

---

## Directory Structure

- `README.md`: This file.
- `requirements.txt`: Python dependencies (e.g., `requests`, `zeroconf`).
- `client.py`: HTTP client wrapper for API endpoints.
- `manager.py`: Host registry manager for `known_hosts.csv`.
- `discovery.py`: Network discovery and peer crawl logic.
- `cli.py`: Command-line interface for debugging and control.

## API Standards

### Error Response Format
All API endpoints must return helpful error messages for 400-series client errors. If a request is malformed (missing fields, wrong types), the server must respond with:
1.  **Explanation**: What went wrong.
2.  **Schema**: What a valid request looks like.

**Example 400 Response:**
```json
{
  "status": "error",
  "message": "Missing required field: 'task_type'",
  "expected_format": {
    "task_type": "string",
    "priority": "int (optional)",
    "params": {}
  },
  "example": {
    "task_type": "SpectrumScan",
    "params": { "freq_start": 433.0, "freq_end": 434.0 }
  }
}
```

## API Reference

### 1. General Status
*   **Endpoint:** `/api/status`
*   **Method:** `GET`
*   **Description:** Returns the complete system state including health, logs, peers, AND current queue execution details.
    *   **Response Example**:
    ```json
    {
      "uptime": 12345,
      "heap_free": 150000,
      "time": 1768435200,
      "ntp_sync": true,
      "timezone": "America/Los_Angeles",
      "plugin": "Scanner",
      "clusterName": "Alpha",
      "status": "Working: Scanner",
      "task": "Broadband Sweep",
      "queue": {
          "depth": 2,
          "current": {
              "id": "uuid-1234",
              "plugin": "Scanner",
              "task": "Broadband Sweep",
              "elapsed": 500,
              "duration": 5000
          }
      },
      "led": { "power": true, "color": { "r": 0, "g": 255, "b": 0 } },
      "peers": [ ... ],
      "logs": [ ... ]
    }
    ```

### 2. Configuration
*   **Endpoint:** `/api/config`
*   **Method:** `GET` / `POST`
*   **Description:** Get or update persisted configuration settings (stored in NVS).
    *   **Example GET Response**:
    ```json
    {
      "ssid": "MyWiFi",
      "hostname": "allseeingeye-a1b2c3",
      "pass": "******",
      "cluster": "Default",
      "description": "Kitchen Node",
      "peer_ignore_hours": 12,
      "timezone": "America/Los_Angeles"
    }
    ```
    *   **Example POST Body**:
    ```json
    {
      "description": "Kitchen Node",
      "hostname": "allseeingeye-a1b2c3",
      "cluster": "Default",
      "timezone": "America/Los_Angeles"
    }
    ```
    *   **Notes**:
        *   Timezone changes apply on reboot.

### 3. Task Management
*   **Endpoint:** `/api/queue`
*   **Method:** `GET`
*   **Description:** Returns the current active task and the pending task queue.

### 4. System Utilities
*   **Endpoint:** `/api/reboot`
*   **Method:** `POST`
*   **Description:** Triggers a system restart. Returns 200 OK immediately, then reboots after 100ms.

### 5. Logging
*   **Endpoint:** `/api/logs`
*   **Method:** `GET`
*   **Description:** Returns the runtime log buffer (Tail). Rotates when full.
*   **Endpoint:** `/api/logs/head`
*   **Method:** `GET`
*   **Description:** Returns the first 50 logs from the boot sequence (Head). Valid until reboot.

### 6. LED Control
*   **Endpoint:** `/api/led`
*   **Method:** `GET`
*   **Params:** `r` (0-255), `g` (0-255), `b` (0-255)
*   **Description:** Updates the global LED color state. This will only be visible if the LED is powered ON by the user.
    *   Example: `/api/led?r=255&g=0&b=128`

*   **Endpoint:** `/api/led/on`
*   **Method:** `POST`
*   **Description:** Enables physical LED output (User Override).

*   **Endpoint:** `/api/led/off`
*   **Method:** `POST`
*   **Description:** Disables physical LED output (User Override).

### 7. Discovery
*   **Endpoint:** `/api`
*   **Method:** `GET`
*   **Description:** Self-discovery endpoint listing all available routes.

