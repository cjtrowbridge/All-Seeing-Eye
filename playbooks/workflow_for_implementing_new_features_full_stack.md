# Playbook: Workflow for Implementing New Features (Full Stack)

*Status: Stable*

## Objective
This playbook defines the "Full Stack" development process for adding a new capability to the All Seeing Eye. It ensures that no feature exists in the firmware without being accessible via the API and documented for agents.

## Prerequisites
*   Development Environment set up (VS Code, Arduino CLI, Python).
*   Understanding of the `Kernel` -> `Plugin` architecture.

## Step-by-Step Instructions

1.  **Define the Feature Contract**
    *   **Action**: Decide on the API endpoint first.
    *   *Example*: `POST /api/led/blink` -> `{"duration": 1000}`.
    *   **Document it**: Add to `api/README.md` *immediately*.

2.  **Implement Firmware Logic (C++)**
    *   **File**: `firmware/AllSeeingEye/src/Kernel.cpp` (or a specific Plugin).
    *   **Action**: Add the handler in `setupWebServer()`.
    *   *Code*: `server.on("/api/led/blink", HTTP_POST, ...)`
    *   **Logging Requirement**: Every API handler must log request receipt and any errors, plus all significant state changes and success paths (so the feature is observable in `/api/logs` and `/api/status`).
    *   **Test**: Compile and deploy to one node. Verify with `curl` or Postman.

3.  **Implement Python Client Wrapper**
    *   **File**: `api/client.py`.
    *   **Action**: Add a method to the `EyeClient` class.
    *   *Code*:
        ```python
        def blink_led(self, duration_ms=1000):
            return self._post("/api/led/blink", {"duration": duration_ms})
        ```

4.  **Expose in the CLI (Required)**
    *   **File**: `api/cli.py`.
    *   **Action**: Every API endpoint must be callable from the CLI for debugging and verification.
    *   **Rule**: If the CLI already supports generic `get`/`post`, add a documented CLI example for the new endpoint.
    *   **Why**: Field operations rely on CLI parity when WebUI is unavailable.

5.  **Update Agent Playbooks (If needed)**
    *   If this feature changes how we deploy or troubleshoot (e.g., a new "Safe Mode"), create or update the relevant playbook in `playbooks/`.

6.  **Final Verification**
    *   Run a Python script calling the new method.
    *   Run the CLI command for the same endpoint.
    *   Observe the physical device response.
    *   Check `AGENTS.md` and `README.md` to ensure they are up to date.

## Verification
*   Current Code matching Documentation.
*   Feature works via Desktop API.
*   Feature works via CLI (debug parity).
*   Feature works via Web UI (if applicable).
