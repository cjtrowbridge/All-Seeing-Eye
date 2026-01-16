# Playbook: Developing Python API Client Infrastructure

*Status: Active*

## Objective
This playbook establishes the strict engineering standards and implementation patterns for building the Python API Client and CLI tools. It complements the roadmap in `api/README.md` by defining *how* code must be written to ensure observability, robustness, and architectural consistency.

## 1. Core Engineering Standards

*   **Single Source of Truth**: The `known_hosts.csv` file is the only persistence layer.
*   **Fail Fast, Fail Loud**: Network errors must be caught and reported clearly to the user/agent. Do not silently swallow exceptions.
*   **Type Safety**: All new classes and methods must use Python 3.9+ Type Hints.
*   **CLI Parity Required**: Every API endpoint must be callable from the CLI for debugging and verification.

## 2. Implementing Host Management (`HostManager`)

When implementing logic to manage the node registry, adhere to these rules to prevent data corruption and state drift.

### Concurrency & Persistence
*   **Atomic Interaction**: The `known_hosts.csv` file should be read fresh before any modification and written back immediately.
*   **Schema Enforcement**:
    *   **Header Row**: `ip_address,hostname,description,last_seen_timestamp` MUST be present.
    *   **Ordering**: The file is ordered. Index 0 is special (Default Host). New hosts should be appended unless a sorting logic is explicitly requested.
*   **Deduplication**:
    *   Primary Key is **IP Address**.
    *   Start/Update Logic:
        *   If IP exists: Update `hostname` (if changed), `description` (if provided), and `last_seen` (always).
        *   If IP new: Append new row.

### Error Handling
*   **Missing File**: If `known_hosts.csv` is missing, the manager must initialize an empty structure (or trigger discovery) rather than crashing.
*   **Corrupt File**: If the CSV is malformed, log a warning to stderr and backup the corrupt file (e.g., `known_hosts.csv.bak`) before creating a fresh one.

## 3. Implementing Network Discovery

Discovery is the mechanism for populating the Host Manager.

### Protocol Constraints
*   **Timeouts**: Network scans must have strict timeouts (e.g., 2-5 seconds) to prevent the CLI from hanging indefinitely.
*   **Passive vs Active**:
    *   **mDNS**: Preferred. Listen for `_http._tcp.local.` services.
    *   **Peer Crawl**: When connecting to a node, read its `/api/status`. If the `peers` array contains unknown IPs, add them to the manager.

### Integration Rule
*   **Auto-Trigger**: The API Client should automatically trigger a scan ONLY if the `known_hosts.csv` is empty or missing. It should *not* auto-scan on every command (performance cost).

## 4. Implementing the CLI (`cli.py`)

The CLI is the interface for both humans and agents. Consistency in I/O is critical for automation.

### Input Arguments
*   **Target Selection**:
    *   `--host [string]`: Select by IP address, hostname, or description (quote names with spaces).
    *   *Default*: If omitted, use the top row in `known_hosts.csv`.
*   **Output Formatting**:
    *   `--json`: Force output to be raw JSON (for piping to `jq` or agents).
    *   *Default*: Human-readable tables for lists, pretty-printed JSON for API responses.

### Implementation Pattern (Command Router)
Do not write logic in the `if __name__ == "__main__":` block.
1.  Parse Arguments.
2.  Instantiate `HostManager` and resolve the target IP.
3.  Instantiate `EyeClient` with the resolved IP.
4.  Dispatch to a handler function (e.g., `handle_get(client, endpoint)`).

### Endpoint Coverage Rule
*   **Requirement**: Every API endpoint documented in `api/README.md` must be usable via the CLI.
*   **Implementation**: Either add a dedicated CLI command/flag or document the generic `get`/`post` usage for that endpoint.

### Output Streams
*   **Success Data**: Print to `stdout`.
*   **Logs/Errors/Debug**: Print to `stderr`.
    *   *Reason*: Allows agents to capture valid JSON from `stdout` even if debug logs are printed to `stderr`.

## 5. Implementing the API Wrapper (`EyeClient`)

### URL Handling
*   **Normalization**: The client must handle endpoints with or without leading slashes.
    *   `client.get("api/status")` and `client.get("/api/status")` must work identically.

### Exception Safety
*   **Connection Errors**: Wrap `requests.exceptions.ConnectionError`. Return a clear `ConnectionRefused` or `HostUnreachable` status, possibly prompting the user to remove the host from the registry.
*   **JSON Parsing**: Wrap `json.JSONDecodeError`. If the server returns non-JSON (e.g., an nginx error page), return the raw text content for debugging.

## 6. Verification Checklist
Before committing changes to the API Client:
1.  **Fresh Install**: Delete `known_hosts.csv`. Run `cli.py --list`. Does it discover credentials?
2.  **Default Target**: Run `cli.py get /api/status`. Does it hit the first host?
3.  **Error Case**: Run `cli.py --host "does-not-exist" get /api/status`. Does it fail gracefully with an error message?
4.  **Logging**: Are network operations visible when debug mode is enabled?
5.  **CLI Parity**: Can the CLI invoke every endpoint in `api/README.md`?
