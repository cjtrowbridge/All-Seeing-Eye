# All Seeing Eye - Desktop API Client

This directory contains Python scripts and documentation for interacting with the All Seeing Eye cluster from a desktop environment.

## Overview

The All Seeing Eye nodes expose a RESTful JSON API for status monitoring, configuration, and task management. This toolkit provides a convenient Python interface to discover nodes, query their state, and issue commands to the fleet.

## Features

- **Discovery:** Tools to find nodes on the network (via mDNS or IP scanning).
- **Status Monitoring:** Fetch real-time status (Task, Uptime, Connected Peers, Signal Strength).
- **Fleet Management:** methods to reboot, update configuration, or deploy firmware (deprecated/experimental).
- **Data logging:** Scripts to poll the cluster and log state over time.

## Directory Structure

- `README.md`: This file.
- `requirements.txt`: Python dependencies (e.g., `requests`, `zeroconf`).
- `client.py`: Main library class for interacting with a single node or the cluster.
- `examples/`: Example scripts showing how to use the client.

## Getting Started

1. Install dependencies:
   ```bash
   pip install -r requirements.txt
   ```

2. Run a status check:
   ```bash
   python check_cluster.py
   ```

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
*   **Description:** Returns the complete system state. To reduce network load, this endpoint aggregates:
    *   System Health (Uptime, Heap, Build ID)
    *   Peer List (Discovered neighbors)
    *   Recent Logs (Last 50 entries)
    *   Active Task Status

### 2. Task Management
*   **Endpoint:** `/api/queue`
*   **Method:** `GET`
*   **Description:** Returns the current active task and the pending task queue.

### 3. System Utilities
*   **Endpoint:** `/api/reboot`
*   **Method:** `POST`
*   **Description:** Triggers a system restart. Returns 200 OK immediately, then reboots after 100ms.

### 4. Logging
*   **Endpoint:** `/api/logs`
*   **Method:** `GET`
*   **Description:** Returns the runtime log buffer (Tail). Rotates when full.
*   **Endpoint:** `/api/logs/head`
*   **Method:** `GET`
*   **Description:** Returns the first 50 logs from the boot sequence (Head). Valid until reboot.

### 5. LED Control
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

### 6. Discovery
*   **Endpoint:** `/api`
*   **Method:** `GET`
*   **Description:** Self-discovery endpoint listing all available routes.

