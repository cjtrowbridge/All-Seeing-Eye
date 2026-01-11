# All-Seeing-Eye Firmware Architecture

This document outlines the **ASE-OS (All-Seeing-Eye Operating System)**, the firmware architecture operating on the ESP32-S3.

## 1. High-Level Concept: "Micro-Kernel with Pluggable Modules"

Instead of a monolithic loop, the firmware is structured as a lightweight "Kernel" that manages system resources (Wi-Fi, API, Storage) and runs exactly one "Active Plugin" (User Mode) at a time.

### The Stack layers
1.  **Hardware Abstraction Layer (HAL)**: Wrappers for the Radio (CC1101), LEDs, and File Systems.
2.  **System Services (Kernel)**: Wi-Fi/Network Manager, Async Web Server, Configuration Manager, Plugin Manager.
3.  **Application Layer (Plugins)**: Isolated logic containers (e.g., `RSSIScanner`, `PacketSniffer`, `BeaconEmitter`).

---

## 2. Hardware Utilization (ESP32-S3)

We utilize the specific strengths of the ESP32-S3-WROOM-1-N16R8:

*   **Dual-Core Concurrency**:
    *   **Core 0 (System)**: Handles Wi-Fi, API serving, Dashboard, and OTA.
    *   **Core 1 (Radio)**: dedicated exclusively to the `Plugin Loop` (Scanning/Radio ops) for uninterrupted precision.
*   **Memory Management**:
    *   **16MB Flash (NVS/LittleFS)**: Stores the OS code, Configuration files, and the Static Web Dashboard files.
    *   **8MB PSRAM (OPI)**: Serves as the high-speed **Data Ring Buffer**. Scan data is written here first to avoid wearing out Flash storage.
*   **Resource Monitoring**:
    *   The API reports real-time statistics for: Heap Free, PSRAM Free, Flash Usage, Uptime, and Core Temperature (if available).

---

## 3. Core Components

### A. Configuration Manager (Persistence)
*   **Storage**: **Preferences API (NVS)** for rapid key/value storage; **LittleFS** for larger structures.
*   **Central Registry**: Holds system-wide settings (SSID, Hostname) and Plugin-specific settings (Frequency, Gain).
*   **Hot-Reloading**: When settings change via API, the Manager updates persistent storage and immediately triggers the Active Plugin's `reload()` method without a system reboot.

### B. The Plugin System (Modes of Operation)
Extensibility is handled via the `ASEPlugin` C++ Interface.

*   **Standard Interface**: Every plugin must implement:
    *   `setup()`: Initialize radio/logic.
    *   `loop()`: The repetitive task (running on Core 1).
    *   `teardown()`: Cleanup before switching.
    *   `getJsonData()`: Return data for the API.
    *   `handleCommand()`: Accept direct API commands.
    *   `getMetadata()`: Return Name, Version, Description.
*   **Default Plugin**: `RSSIScannerPlugin` (The "Eye" functionality).
*   **Switching**: The User selects a plugin via Dashboard -> API Request -> System stops current plugin -> Loads new plugin.

### C. Data Persistence & Buffering
*   **Ring Buffer (PSRAM)**: A circular buffer in RAM is used for high-speed data collection.
*   **API Access**: The API reads directly from this RAM buffer (mutex-protected).
*   **Snapshotting**: On graceful shutdown or user request, the RAM buffer is dumped to `scan_data.json` in LittleFS. On boot, it is reloaded.

### D. The Web Architecture (High Performance)
To ensure the Radio never stutters, we use **Asynchronous** I/O and **Client-Side Rendering**.

1.  **The API (Backend)**: Built on `ESPAsyncWebServer`.
    *   **Response Format**: STRICTLY JSON.
    *   **Discovery**: `GET /api` lists all available endpoints and their descriptions. This list must be kept current with every new feature.
    *   **Endpoints**:
        *   `GET /api`: Index of all endpoints.
        *   `GET /api/status`: System health (RAM/PSRAM/Flash stats), Current Plugin, Uptime.
        *   `GET /api/config`: Returns settings JSON.
        *   `POST /api/config`: Updates settings & hot-reloads.
        *   `GET /api/plugins`: List available plugins.
        *   `POST /api/plugins/switch`: Switch active plugin.
        *   `GET /api/data`: Returns the current dataset from the Active Plugin.
        *   `GET /api/fs`: **File System Index**. Returns a JSON list of all files in LittleFS with sizes and links.
        *   `GET /api/fs/{filename}`: Download specific file content.
2.  **The Dashboard (Frontend)**:
    *   **Technology**: **Pure Vanilla JavaScript (ES6+)**. No heavy frameworks (React/Vue) to map variables or bloat the build.
    *   **Serving**: The HTML/CSS/JS assets are compressed (GZIP) and stored in the **LittleFS** partition.
    *   **Logic**: The ESP32 serves *static files only*. The Browser downloads the app, runs the JS, and fetches JSON from the API to render charts/tables locally.

### E. Logging System ("RAM Flight Recorder")
To preserve flash longevity while still capturing detailed runtime events, logs are handled by a dedicated `Logger` class.
*   **Dual Output**: 
    1.  **Serial**: Real-time output for USB debugging.
    2.  **RAM Ring Buffer**: Stores the last ~20KB (or 200 lines) of logs in Heap/PSRAM.
*   **API Accessibility**: The `/api/logs` endpoint retrieves this buffer, allowing the Web Dashboard to show a "Console" view without needing a physical connection.
*   **Thread Safety**: Protected by Mutex to allow safe logging from both Core 0 (System) and Core 1 (Plugins).

## 4. Expansion & Future-Proofing

*   **Plugin Repository**: The structure allows community contributions by simply adding a new Class file implementing `ASEPlugin`.
*   **Hardware Agnostic**: Radio objects are passed to plugins. Future hardware swaps (e.g., to LoRa SX1262) require HAL updates but minimal Plugin changes.

## 5. Summary Workflow

1.  **Boot**: System loads Config; Core 0 starts Wi-Fi/Web; Core 1 starts Default Plugin (`RSSIScanner`).
2.  **Scan**: Plugin loops on Core 1, writing RSSI/Freq pairs to PSRAM buffer.
3.  **User Connects**: Browser loads `index.html` (from LittleFS).
4.  **Visualization**: Browser JS polls `/api/data` and `/api/status`, rendering the "Eye" visualization.
5.  **Control**: User changes settings in UI. API updates Config Manager. Plugin adjusts Radio instantly.

---

## 6. Implementation Roadmap

This roadmap outlines the step-by-step construction of the ASE-OS to meet all architectural and functional specifications.

### Phase 1: Foundation & The "Kernel"
**Goal**: Establish the Dual-Core OS, File System, and Network capabilities.
- [x] **Project Restructuring**: Convert the single `.ino` into a modular C++ structure (`Kernel.h`, `Config.h`, `HAL.h`).
- [x] **Partition Setup**: Verify and configure the 16MB Flash / 8MB PSRAM partition scheme (ensuring ample LittleFS space).
- [x] **LittleFS Integration**: Implement the file system mount and the **File Indexer API** (`/api/fs` endpoint).
- [x] **Core System Logging**: Implement RAM-based Ring Buffer logging and `/api/logs` endpoint.
- [x] **Web Server Base**: Launch the `ESPAsyncWebServer` on Core 0 (upgraded for ESP32 Core 3.0+ compatibility).
- [x] **OTA Support**: Restore ArduinoOTA for network-based firmware updates.

### Phase 2: Memory & Configuration
**Goal**: Safeguard the hardware and enable persistent settings.
- [x] **Config Manager**: Implement the Preferences-based settings manager (Key-Value store) and the `/api/config` endpoints (GET/POST).
- [x] **Smart Network Boot**: Implemented priority connection logic (Saved Config -> Secrets.h -> AP Mode Recovery).
- [x] **API Discovery**: Implement the `/api` endpoint to list all available routes.
- [ ] **PSRAM Ring Buffer**: implementation of the high-speed circular data buffer in the 8MB PSRAM.
- [x] **Resource Monitor**: Create the `/api/status` endpoint to report real-time RAM, Flash, and uptime stats.

### Phase 3: The Plugin Engine
**Goal**: Build the extensible logic system.
- [ ] **Interface Definition**: Create the `ASEPlugin` abstract base class (Setup, Loop, Teardown, GetJson).
- [ ] **Plugin Manager**: Build the "Switcher" logic that stops one plugin, clears memory, and starts another.
- [ ] **Dummy Plugin**: Create a "SystemIdle" plugin to verify the switching logic and API responses without radio hardware.

### Phase 4: Hardware Drivers & Plugins
**Goal**: Bring the "All-Seeing Eye" to life with concrete radio modes.
- [ ] **Radio HAL**: Integrate the CC1101 library (e.g., RadioLib) behind a hardware abstraction layer.
- [ ] **Power-On Self Test (POST)**:
    -   Initialize CC1101 radio on specific pins (defined in root README).
    -   Verify SPI communication and register access.
    -   Provide clear Serial feedback (Success/Fail) on boot.
- [ ] **Spectrum Explorer Plugin (First Implementation)**: An interactive RX/TX tool for detection and debugging.
    -   **Features**: RSSI Waterfall visualization, Adjustable Frequency Range, Manual Packet Send/Receive.
    -   **Defaults**: Starts at 915MHz (Meshtastic), with buckets aligned to the Meshtastic channel scheme.
- [ ] **RSSIScanner Plugin (Default Boot Mode)**: Implement the core "Eye" mode—autonomous fast spectrum scanning and writing to the PSRAM buffer.
- [ ] **LED Manager**: Integrate status lighting (Yellow/Purple/Blue) into the Kernel and Plugin events.

### Phase 5: The Web Dashboard (Frontend)
**Goal**: Create the user interface.
- [ ] **Frontend Skeleton**: Create the pure HTML/JS project structure.
- [ ] **API Client**: Write the JS functions to poll `/api/data` and `/api/status`.
- [ ] **Visualization**: Implement the Canvas/Chart.js rendering of RSSI data.
- [ ] **Configuration UI**: Create forms to read/write settings via `/api/config`.
- [ ] **Build Pipeline**: Create a script to GZIP frontend files and upload them to LittleFS.

### Phase 6: Expansion & Optimization
**Goal**: Future-proofing.
- [ ] **API Documentation**: Finalize OpenAPI/Swagger definition for the JSON API.
- [ ] **Error Handling**: Implement Watchdog Timers (WDT) to reboot if a plugin freezes.
- [ ] **Secrets Management**: Ensure strict separation of `secrets.h` from the public repo.
