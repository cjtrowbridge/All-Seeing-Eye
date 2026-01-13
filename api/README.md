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

(Detailed API endpoint documentation will go here)
