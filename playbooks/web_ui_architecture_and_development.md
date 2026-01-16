# Playbook: Web UI Architecture & Development

## Overview
The All Seeing Eye Web Interface is a Single Page Application (SPA) embedded directly into the ESP32 firmware. It is designed for resource-constrained environments (no external CDNs, minimized payloads) while offering a rich 3-column desktop layout.

## Architecture

### 1. File Structure
*   **Source**: `firmware/web/index.html` (The **ONLY** editable source file).
*   **Build Artifact**: `firmware/AllSeeingEye/src/WebStatic.h` (Generated Gzip + Hex dump).
*   **Deployment**: The artifact is compiled into the C++ binary and served from flash memory.

### 2. Layout System
The UI uses a Responsive Grid System:
*   **Mobile (< 1000px)**: Single column view with bottom navigation tabs (`Cluster`, `Tasks`, `Monitor`).
*   **Desktop (>= 1000px)**: Three simultaneous columns:
    1.  **Environment (Left)**: Network topology, Cluster Tree, and Logs (Tail/Head).
    2.  **Controls (Center)**: 
        *   **Task List**: Catalog of missions grouped by Plugin.
        *   **Device Config**: Identity and hardware settings.
    3.  **Workspace (Right)**: Task configuration forms, execution progress, and results.

### 3. Task Catalog & Dynamic Forms
*   **Source of Truth**: The firmware (`PluginManager.cpp`) defines the catalog and form schemas.
*   **Flow**:
    1.  WebUI fetches `GET /api/task`.
    2.  Loops through the list, creating buttons.
    3.  **Grouping**: Tasks are grouped by the `plugin` field (e.g., "Spectrum Analyzer"). The UI auto-capitalizes and spaces these keys.
    4.  **Selection**: When clicked, the UI generates a form based on the `inputs` array defined in the API response.

## Development Workflow

### Making Changes
1.  **Edit HTML/JS/CSS**: Modify `firmware/web/index.html`.
2.  **Pack Assets**: Run the Python packer script.
    ```powershell
    cd firmware
    .\pack_web.py
    ```
    *This generates `WebStatic.h`. If you skip this, your changes will NOT appear on the device.*
3.  **Compile & Upload**:
    ```powershell
    .\upload_ota.ps1
    ```

### Best Practices
*   **Single File**: Keep everything (CSS, JS, HTML) in `index.html`. External requests (fonts, scripts) will fail on offline networks.
*   **No Frameworks**: Use Vanilla JS. React/Vue are too heavy for the 4MB flash / 300KB RAM constraints if not carefully managed.
*   **Polling vs Sockets**: Prefer polling `/api/status` (2s interval) over independent requests. The ESP32 struggles with concurrent TCP connections.

## Troubleshooting
*   **ReferenceError in Console**: You likely deleted a core function (e.g., `runForceSimulation`) while refactoring. Use `git restore` and apply changes incrementally.
*   **Changes not showing**: You forgot to run `pack_web.py` before uploading.
