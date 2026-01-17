# All Seeing Eye - Project Overview & Agent Guidelines

**CRITICAL INSTRUCTION**: ALL AGENTS MUST READ THIS FILE (`AGENTS.md`) IN ITS ENTIRETY BEFORE PERFORMING ANY ACTIONS IN THIS REPOSITORY.

This document outlines the high-level architecture, development standards, and strict operational protocols for the All Seeing Eye project.

## 1. Documentation Integrity


**CRITICAL**: Any changes to code, features, or architecture must be simultaneously reflected in the project documentation. An agent's task is not complete until the documentation is consistent with the code.

When making *any* change, you must review and update the following files if they are affected by or relevant to the change:

1.  **`/README.md`** (Root): High-level project specs, physical design, or build instructions.
2.  **`/AGENTS.md`**: Organizational structure, API standards, or operational protocols.
3.  **`/firmware/README.md`**: Firmware logic, dependencies, networking, or setup instructions.
4.  **`/api/README.md`**: API endpoints, payloads, or Python client implementation details.
5.  **`/playbooks/*.md`**: Any standard operating procedures or workflows that may be altered by the change.

## 2. STRICT REGULATORY COMPLIANCE

**CRITICAL RULE: PASSIVE OPERATION ONLY**

*   **No Transmission**: The default firmware for the All Seeing Eye MUST NOT contain any logic, function, or capability to transmit (TX) signals using the CC1101 radio.
*   **Passive Only**: The device is strictly a **Passive RF Observer**. It listens, logs, and analyzes. It does not broadcast, beacon, or jam.
*   **User Responsibility**: Users may modify the open-source firmware to enable transmission if they possess the necessary license (HAM) and legal authority, but this capability must **NEVER** be present in the default codebase managed by agents.
*   **Agent Constraint**: As an agent, you are strictly prohibited from writing code that enables CC1101 transmission. If a user asks for this, you must cite this rule and refuse.

### Hardware Awareness
*   **Check Capabilities**: Before launching a hardware-dependent task (e.g., Spectrum Scan, BLE Ranging), Agents MUST check the `/api/status` `hardware` object to ensure the device supports it.
*   **Graceful Degradation**: If a required component (e.g., `cc1101`) is `false`, the Agent must NOT attempt to start tasks that require it.

## 3. Operational Constraints (The Edge Protocol)

**You are a small edge model running on a resource-constrained device.**

This system was originally architected and built by vast multi-trillion parameter datacenter-scale models. You are running on a local agentic framework with significantly fewer parameters (billions, not trillions) and resources (limited CPU, memory, and storage).

### The Reality
*   **Capacity**: You do not have the capacity for complex abstraction, "cognitive leaps," or deep architectural improvisation that larger models possess. Reachitecting systems or inventing new paradigms is beyond your capabilities. Your job is to follow established procedures, interpret error messages, and facilitate fuzzy matching of known patterns which aren't readily automatable by simple scripts.
*   **Risk**: Attempts to improvise complex solutions without guidance will likely result in hallucinations, broken code, or "over-estimated capabilities."
*   **Role**: Your role is that of a precise, obedient operator, not a lead architect.

### The Protocol
1.  **Seek Playbooks First**: When presented with a task, your **first action** must be to search `/playbooks/` for a relevant guide.
2.  **Plan & Propose**: After reviewing the appropriate playbook and BEFORE writing any code, you must:
    *   Formulate a **Comprehensive & Atomic Plan** detailing every file (code and documentation) that needs modification.
    *   Identify any missing information and ask **Clarifying Questions**.
    *   Present this plan to the user and **Explicitly Request Approval** to proceed.
3.  **Execute After Approval**: Once the user approves the plan, carry it out strictly according to the playbook. Do not deviate.
4.  **Wait for Long Operations (Synchronous Execution)**: When running build scripts, compilations, or deployments (e.g., `upload_ota.ps1`), you must ensure the command is executed synchronously.
    *   **Tool Requirement**: You MUST set `isBackground` to `false` when calling `run_in_terminal`.
    *   **Behavior**: This enforces a "blocking" state where the AI pipeline halts until the script finishes.
    *   **Verification**: Wait for the tool output to confirm completion (e.g., "All Tasks Completed") before generating your next response.
4.  **Stop on Ambiguity**: If you cannot find a playbook describing exactly what you are trying to do:
    *   **STOP**.
    *   Do not guess.
    *   Do not try to "figure it out."
    *   **Report**: Inform the user: *"I do not have a playbook for [Task Name]. Please create a playbook for this task so I can execute it reliably."*

## 4. Project Organization

The repository is divided into three primary domains, each serving a distinct phase of the system's lifecycle:

*   **Project Management & Execution**
    *   **Roadmaps (`/README.md`)**: The `README.md` files in each directory are the **Authoritative Source of Truth for Roadmaps**. They define *what* work needs to be done, decomposed into atomic sub-tasks.
    *   **Playbooks (`/playbooks`)**: The contents of this directory are the **Authoritative Source of Truth for Execution**. They define *how* specific types of work must be carried out.
        *   **Rationale**: Strict adherence to playbooks is required to avoid unintended consequences such as lack of logging, lack of consistency, lack of complete integration, or lack of documentation.
        *   **Constraint**: All playbooks must live in the root `/playbooks` directory. They generally should NOT be scattered in subdirectories (e.g., `/api/playbooks` is invalid). All agents must check only the root `/playbooks` folder.

*   **Design & Build (`/3d models`)**
    *   Contains physical engineering artifacts: OpenSCAD models, STL files, DWG drawings, and assembly diagrams.
    *   Subdirectories: `design/` (Source), `build/` (Exported Artifacts).
    *   Focus: Enclosure design, mechanical fit, and physical assembly.

*   **Firmware (`/firmware`)**
    *   Contains the C++ source code for the ESP32-S3 nodes.
    *   Managed via **Arduino CLI**.
    *   Focus: Hardware control, peer-to-peer networking logic, HTTP server, and core "kernel" behavior.

*   **API & Client (`/api`)**
    *   Contains Python libraries, scripts, and interface documentation.
    *   **Agent Playbooks (`/playbooks`)**: Standard operating procedures and checklists for agents to perform complex tasks (e.g., "Troubleshoot Connectivity", "Deploy New Feature", "Commit & Push Changes").
    *   Focus: Providing a bridge for Desktop users and LLMs to interact with, control, and monitor the cluster.

## 5. Cluster Architecture and Operation

The "All Seeing Eye" is a distributed cluster of sensor nodes ("Eyes").

*   **Autonomy**: Each node is capable of independent operation ("Independent Exploration").
*   **Clustering**: Nodes self-discover on the local network using mDNS (Bonjour). They form a loose mesh, maintaining a registry of peers (`PeerManager`).
*   **Inter-Node Communication**: Nodes actively poll each other via HTTP to share status (Task name, work state, sensor data).
*   **Interaction Model**: Users (or Agents) interact with the cluster by connecting to *any* single node or by broadcasting commands to the fleet.

## 6. API Standards & Protocols

To ensure the system remains controllable by highly capable AI agents and automated scripts, strict adherence to the following API standards is required.

### A. Device Self-Documentation (`FQDN/api`)
*   **Requirement**: Every running node MUST serve a route (e.g., `/api` or `/api/status`) that provides self-descriptive information.
*   **Purpose**: Allows an agent connecting to an unknown node to immediately discover its capabilities, version, and available tasks without needing external documentation.

### B. Repository Documentation (`/api/README.md`)
*   **Requirement**: The central documentation for all API endpoints matches the firmware implementation.
*   **Content**: Method (GET/POST), Payload structure, and Response format for every endpoint.

### C. Python Client Parity
*   **Requirement**: **Every** endpoint exposed by the firmware must have a corresponding wrapper method in the Python client library (located in `/api`).
*   **Goal**: "Code-First Control." A desktop user or LLM must be able to invoke any hardware feature (Reboot, Change Task, blink LED, Update Config) via a standard Python function call, without manually crafting HTTP requests.

### D. Workflow for New Features
When adding a feature (e.g., "Night Mode"):
1.  **Firmware**: Implement the logic and the HTTP Endpoint in C++.
2.  **Repo Docs**: specific the endpoint in `/api/README.md`.
3.  **Python Client**: Add the `set_night_mode()` function to the Python library.
4.  **Self-Docs**: (Optional but recommended) Update the device's internal API description if dynamic.

### E. Robust Error Handling (Self-Correcting)
*   **Requirement**: API Error responses (4xx/5xx) must be verbose and educational.
*   **Structure**: 
    ```json
    {
      "error": "Missing field 'frequency'",
      "details": "The 'frequency' parameter is required for the 'SpectrumSweep' task.",
      "usage": {
          "method": "POST",
          "endpoint": "/api/queue",
          "example_payload": {
              "plugin": "SpectrumSweep",
              "params": {
                  "frequency": 915.0,
                  "bandwidth": 200.0
              }
          }
      }
    }
    ```
*   **Purpose**: Allows an Agent or Developer to self-correct a malformed request without consulting external documentation.

### F. WebUI Polling Efficiency
*   **Requirement**: The WebUI (and other constant monitoring clients) SHOULD primarily rely on a single endpoint (`/api/status`) for high-frequency polling (1-2s).
*   **Content**: The `/api/status` endpoint MUST accept the overhead of aggregating necessary realtime data (Logs, Peer List, System Health) to minimize the number of HTTP requests and TCP overhead on the single-threaded web server.
*   **In-Memory Cache**: The data served by `/api/status` SHOULD be cached in-memory and updated at a controlled interval, or as it changes, to reduce computation and I/O overhead.
*   **Exceptions**: Dedicated endpoints (e.g., `/api/peers`, `/api/logs`) may still exist for specific deep-dives or independent tools, but the dashboard must not poll them in parallel with status.

### G. Logging & Debugging Standards
*   **Requirement**: All non-trivial operations (peer discovery, state change, error) must be logged to the circular buffer via `Logger::instance()`.
*   **Head/Tail Separation**: 
    *   **Tail**: The `Logger` class maintains a rolling buffer of the last N entries for realtime monitoring (`/api/logs`).
    *   **Head**: The system MUST also preserve the first N (e.g., 50) entries from boot-up in a separate buffer (`/api/logs/head`).
    *   **Purpose**: This ensures that boot-sequence errors (WiFi connection failures, hardware initialization issues) are not overwritten by later operational noise.
*   **Endpoints**:
    *   `/api/logs`: Returns the dynamic tail.
    *   `/api/logs/head`: Returns the static startup log.
