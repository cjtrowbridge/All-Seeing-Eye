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

*   **BLE Ranging (Latest Scan)**:
    ```bash
    python cli.py --host "eye-beta" --ble-ranging
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
| `/api/task` | GET | Task catalog with input schemas |
| `/api/task/{taskId}` | POST | Execute a task with parameters |
| `/api/cluster/deploy` | POST | Stage a task payload for the cluster |
| `/api/cluster/start` | POST | Start the staged task cluster-wide |
| `/api/report` | GET | Aggregated task report across cluster |
| `/api/reboot` | POST | Reboot the device |
| `/api/ranging/ble` | GET | Latest BLE ranging scan results |

## Data Models

### Peer Object
Returned in `/api/peers` and the `peers` array of `/api/status`.

```json
{
  "hostname": "allseeingeye-4ae214",
    "description": "lab node",
    "ip": "192.168.1.105",
    "cluster": "Default",
    "status": "Ready",
    "task": "System Idle",
    "desired_task": {
        "id": "spectrum/scan",
        "params": { "start": 902.0, "stop": 928.0 }
    },
    "start_requested": false,
    "online": true,
    "lastProbe": 1705351234,
    "ble_rssi": [-85, -82, -80, -99, -84],
    "ble_dist_m": 3.42
}
```

*   `ble_rssi`: Array of last 5 Bluetooth RSSI measurements. `-99` indicates the peer was not seen during that scan window.
*   `ble_dist_m`: Estimated distance in meters based on Path Loss model. `null` if no recent data.

### Task Catalog Entry
Returned by `GET /api/task`.

```json
{
    "id": "spectrum/scan",
    "name": "Band Scan",
    "description": "Standard sweep, returns power levels.",
    "plugin": "Spectrum Analyzer",
    "link": "/api/task/spectrum/scan",
    "inputs": [
        {
            "name": "start",
            "label": "Start Frequency (MHz)",
            "type": "number",
            "default": 905,
            "step": 0.1,
            "required": true
        },
        {
            "name": "stop",
            "label": "Stop Frequency (MHz)",
            "type": "number",
            "default": 928,
            "step": 0.1,
            "required": true
        },
        {
            "name": "bandwidth",
            "label": "Channel Bandwidth (kHz)",
            "type": "number",
            "default": 500,
            "step": 1,
            "required": true
        },
        {
            "name": "power",
            "label": "Broadcast Power (dBm)",
            "type": "number",
            "default": -1,
            "min": -30,
            "max": 11,
            "step": 1,
            "required": true
        }
    ]
}
```

### Spectrum Report Payload
Returned under `/api/report.nodes[*].report` when `spectrum/scan` is active.

```json
{
    "task_id": "spectrum/scan",
    "start_mhz": 905,
    "stop_mhz": 928,
    "bandwidth_khz": 500,
    "power_dbm": -1,
    "step_mhz": 0.5,
    "points_count": 255,
    "points_max": 255,
    "points": [
        { "freq_mhz": 905.0, "rssi_dbm": -78.2 },
        { "freq_mhz": 905.5, "rssi_dbm": -79.1 }
    ],
    "iterations": 12,
    "last_loop_ms": 123456
}
```

### CC1101 Safety Limits
The CC1101 limits are enforced in firmware (`HAL`) and reflected in the task input schema.

- Frequency bands: 300–348 MHz, 387–464 MHz, 779–928 MHz
- Bandwidth: 58–812 kHz
- Power: -30 to +10 dBm
- Defaults: 905–928 MHz, 500 kHz, -1 dBm

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
      "desired_task": {
          "id": "spectrum/scan",
          "params": { "start": 902.0, "stop": 928.0 }
      },
      "start_requested": false,
      "geolocation": {
          "state": "init",
          "fix": "none",
          "confidence": 0.0,
          "last_updated": 0,
          "motion": {
              "stationary": false,
              "speed_mps": 0.0,
              "heading_deg": 0.0,
              "variance": 0.0
          },
          "position": null,
          "relative": null,
          "sources": []
      },
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
                            "ble_ranging": {
                                    "enabled": true,
                                        "scan_interval_ms": 10000,
                                        "scan_window_ms": 200,
                                        "advertise_interval_ms": 1000,
                                        "last_scan": 0,
                                        "service_uuid": "180f6f62-2b31-4307-b353-9d115e5c707d",
                                        "local_bssid": "f4:12:fa:01:02:03",
                                    "peers": [],
                                    "bssids": []
                        },
      "led": { "power": true, "color": { "r": 0, "g": 255, "b": 0 } },
      "peers": [ ... ],
      "logs": [ ... ]
    }
    ```

### 2. Cluster Deploy (Status-Driven)
*   **Endpoint:** `/api/cluster/deploy`
*   **Method:** `POST`
*   **Description:** Stage a task for the cluster by publishing it to local `/api/status` as `desired_task`.
*   **Payload Example**:
        ```json
        {
            "task": "spectrum/scan",
            "params": { "start": 902.0, "stop": 928.0 }
        }
        ```

### 3. Cluster Start
*   **Endpoint:** `/api/cluster/start`
*   **Method:** `POST`
*   **Description:** Sets `start_requested=true` in `/api/status` so peers begin execution.
*   **Payload Example**:
        ```json
        {}
        ```

### 4. Cluster Report
*   **Endpoint:** `/api/report`
*   **Method:** `GET`
*   **Description:** Aggregated report containing the active task payload and per-node results.
*   **Response Example**:
        ```json
        {
            "task": { "id": "spectrum/scan", "params": { "start": 902.0, "stop": 928.0 } },
            "nodes": {
                "allseeingeye-a1": { "task": "Spectrum Scan", "report": { "bins": [ ... ] } },
                "allseeingeye-b2": { "task": "Spectrum Scan", "report": { "bins": [ ... ] } }
            }
        }
        ```

### 10. BLE Ranging (Planned)
*   **Endpoint:** `/api/ranging/ble`
*   **Method:** `GET`
    *   **Description:** Read back the most recent BLE ranging scan results (scan cadence aligned to UTC modulo 10s).
        *   **GET Response Example**:
        ```json
        {
            "enabled": true,
            "scan_interval_ms": 10000,
            "scan_window_ms": 200,
            "advertise_interval_ms": 1000,
            "last_scan": 1768435210,
            "service_uuid": "180f6f62-2b31-4307-b353-9d115e5c707d",
            "local_bssid": "f4:12:fa:01:02:03",
            "peers": [
                {
                    "peer_id": "allseeingeye-acde12",
                    "rssi": -58,
                    "distance_m": 4.2,
                    "seen_at": 1768435200,
                    "name": "allseeingeye-acde12",
                    "service_uuid": "180f6f62-2b31-4307-b353-9d115e5c707d"
                }
            ],
            "bssids": [
                {
                    "bssid": "f4:12:fa:01:02:03",
                    "rssi": -72,
                    "tx_power": -8,
                    "seen_at": 1768435210,
                    "payload": "0201060aff4c001005"
                }
            ]
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

