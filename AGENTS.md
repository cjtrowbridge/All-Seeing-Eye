# All Seeing Eye - Project Overview & Agent Guidelines

This document outlines the high-level architecture and development standards for the All Seeing Eye project. It is intended for both human developers and AI agents to understand the system's divisions and operational protocols.

## 1. Documentation Integrity

**CRITICAL**: Any changes to code, features, or architecture must be simultaneously reflected in the project documentation. An agent's task is not complete until the documentation is consistent with the code.

When making *any* change, you must review and update the following files if they are affected by or relevant to the change:

1.  **`/README.md`** (Root): High-level project specs, physical design, or build instructions.
2.  **`/AGENTS.md`**: Organizational structure, API standards, or operational protocols.
3.  **`/firmware/README.md`**: Firmware logic, dependencies, networking, or setup instructions.
4.  **`/api/README.md`**: API endpoints, payloads, or Python client implementation details.
5.  **`/api/playbooks/*.md`**: Any standard operating procedures or workflows that may be altered by the change.

## 2. Project Organization

The repository is divided into three primary domains, each serving a distinct phase of the system's lifecycle:

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
    *   **Agent Playbooks (`/api/playbooks`)**: Standard operating procedures and checklists for agents to perform complex tasks (e.g., "Troubleshoot Connectivity", "Deploy New Feature").
    *   Focus: Providing a bridge for Desktop users and LLMs to interact with, control, and monitor the cluster.

## 3. Cluster Architecture and Operation

The "All Seeing Eye" is a distributed cluster of sensor nodes ("Eyes").

*   **Autonomy**: Each node is capable of independent operation ("Independent Exploration").
*   **Clustering**: Nodes self-discover on the local network using mDNS (Bonjour). They form a loose mesh, maintaining a registry of peers (`PeerManager`).
*   **Inter-Node Communication**: Nodes actively poll each other via HTTP to share status (Task name, work state, sensor data).
*   **Interaction Model**: Users (or Agents) interact with the cluster by connecting to *any* single node or by broadcasting commands to the fleet.

## 4. API Standards & Protocols

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
