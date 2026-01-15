# Developer Protocol & Tips
**Critical instructions for Agent/Devs to avoid build loops:**

1.  **Build System Limitations**: 
    -   Arduino's build system struggles with complex relative paths in `#include`. 
    -   **Rule**: Keep all header files flat in `src/` (e.g., `src/CC1101Plugin.h`, not `src/plugins/radios/CC1101Plugin.h`). Complexity is managed by namespaces, not folders.

2.  **Build Automation**:
    -   **Never** manually run `arduino-cli` commands in the terminal. The syntax is brittle on PowerShell.
    -   **Rule**: Always use `.\upload_ota.bat`. It handles library paths and OTA upload arguments reliably.

3.  **Concurrency (Dual Core)**:
    -   Core 0 (System) and Core 1 (Plugin) share resources.
    -   **Rule**: Always use `xSemaphoreTake` with a timeout (e.g., `pdMS_TO_TICKS(100)`) when accessing shared objects (like the Plugin pointer). Never block indefinitely.

4.  **Logging & Debugging**:
    -   **Rule**: Do NOT create ad-hoc text files (e.g., `output.txt`, `debug.txt`) in the repo. They clutter the git history.
    -   **Rule**: If you must dump output to a file, use the `.log` extension (e.g., `build.log`), as these are globally ignored by `.gitignore`.
    -   **Rule**: Log timestamps use wall-clock time (NTP-synced) and fall back to Unix epoch seconds before sync.

5.  **Regulatory Compliance (NO TX)**:
    -   **Rule**: The CC1101 radio must be operated in **RX (Receive) Mode ONLY**.
    -   **Rule**: Do not commit any code that initializes the radio for transmission (TX) or enables broadcasting. The default firmware is passive.

6.  **WebUI Polling Optimization**:
    -   **Rule**: The WebUI must only poll `/api/status` on a regular interval (e.g., 2s).
    -   **Rule**: `/api/status` must embed the latest **Logs** (last N lines) and **Peer List** to support this single-call architecture.
    -   **Reason**: The ESP32 single-core network stack struggles with concurrent HTTP requests. reducing connection overhead improves responsiveness.

---

# Roadmap

## Phase 1: Core Framework (Completed)
- [x] Basic ESP32-S3 Board Support
- [x] Double-Buffered Plugin Architecture (Core 0 vs Core 1)
- [x] RingBuffer for high-speed logging
- [x] OTA via `espota`
- [x] Webpack pipeline for embedded `index.html`

## Phase 2: Hardware Drivers (Completed)
- [x] CC1101 SPI Integration
- [x] RSSI Reading & Tuning
- [x] Circular Buffer Logging

## Phase 3: Web Dashboard (Completed)
- [x] Basic SPA structure
- [x] WebSocket/Poll-based logs
- [x] **UI Refactor**: Moves stats to footer, fix overlap (Tabbed View).
- [x] **Navbar Enhancements**: Cluster Status with "Sparkle" indicators.

# Web Interface & Topology Map

The Web Dashboard (`/index.html`) includes a live **Topology Map** visualizing the mesh network.

## Legend Reference

| Element | Color | Description |
| :--- | :--- | :--- |
| **Self (Dot)** | `Cyan (#03dac6)` | The node currently hosting the web interface. |
| **Peer (Dot)** | `Purple (#bb86fc)` | A peer node that is currently **Online** and responsive. |
| **Offline (Dot)** | `Gray (#444)` | A peer node that has timed out (stale) or is unresponsive. |
| **LAN Link** | `Blue (#2196F3)` | A direct network connection (WiFi or Wired) established via mDNS discovery. |
| **Ranging** | `Red (#cf6679)` | (Planned) A physical RF ranging measurement between nodes. |

## Interaction

*   **Click Node**: Navigate to that node's WebUI (`http://[hostname].local`).
*   **Hover**: View tooltip (currently hostname).
*   **Physics**: The map uses a force-directed graph (d3-style) where nodes repel each other but are bound by links.

# Roadmap Continued

## Phase 4: VLBI Clustering (Backend Completed)
- [x] **Core Logic**: Default Kernel state is now `SystemIdle` (POST -> Idle).
- [x] **Unique Identity**:
    -   Hostnames: `allseeingeye-{hexid}.local` (from MAC address).
- [x] **Time Synchronization (Foundation)**:
    - [x] **SNTP Integration**: 
        - [x]  Configure `configTime` with `pool.ntp.org` and proper timezone handling (Los Angeles).
        - [x]  Implement `Kernel::isTimeSynced()` validator (Checks Year > 2025).
        - [x]  Enable time zone configuration via `/api/config` (string, e.g., "America/Los_Angeles") and persist in NVS
        - [x]  Make timezone editable in webui under device tab.
    - [x] **API Visibility**: 
        - [x]  Add `time` (Unix Epoch) and `ntp_sync` (bool) to `/api/status`.
    - [x] **Drift Management**: 
        - [x]  Configure re-sync interval (1hr).
- [x] **Discovery Protocol**:
    -   **Zero-Conf**: mDNS (`_allseeingeye._tcp`) auto-discovery.
    -   **Subnet Scanning**: Automatically scans local /24 subnet (1-254) if isolated.
    -   **Passive Discovery**: Intercepts requests to `/api/peers` to find new neighbors.
    -   **Negative Caching**: Ignores non-peer IPs for 12h to reduce network noise.
- [x] **Cluster Management**:
    -   Nodes advertise `cluster` text record in mDNS.
    -   Clusters visualized in Web UI Tree View.
    -   Configurable **Cluster Name** and **Ignore Hours**.

## Phase 5: Radio Task Queue Architecture (Partial)
- [x] **Task Object & Scheduler**: Abstract `RadioTask` struct and `Scheduler` FIFO class.
- [x] **Startup Stages**: Implement POST -> Infrastructure -> Startup Queue -> App Loop lifecycle.
- [ ] **Startup Tasks**:
    - [x] `RadioHardwareTest`: Validate CC1101 SPI.
    - [ ] `GeolocationScan`: Looks at different possible approaches and applies confidence over time to build a location fix. Tries to identify evidence of movement or stationarity and report this to apps and plugins that need to know the location and vector of the node in order to do their tasks.
        - [ ] **Precision Approach (GPS)**:
             - [ ] If a GPS module is connected, use that.
        - [ ] **Native Approach (No GPS)**
             - [ ] **Relative Positioning (MDS)**: Broadcast/Receive packets between nodes in the cluster to build a relative distance map using RSSI.
             - [ ] **BLE Ranging**: Use the ESP32's Bluetooth radio to range-find with peers and static beacons. Provides high-resolution close-range data (<20m) and helps detect movement vs. stationarity.
             - [ ] **Absolute Anchoring (WiFi)**: Scan nearby Wifi BSSIDs and query Geolocation API (15-30m absolute), then use the RSSI to each node in the cluster to determine the distance and relative position of our nodes versus the BSSIDs in order to locate all of our nodes.
        - [ ] **VLBI / TDOA (Macroscopic)**:
             - [ ] If the nodes in the cluster are sufficiently far apart, TDOA may be a feasible approach to triangulate position based on signal arrival times from known broadcast origins (e.g., FM Radio, TV Towers, Cell Towers).
        - [ ] **Mesh-Assisted Localization**:
             - [ ] Passive listening for single-hop packets from Meshtastic nodes with GPS.
             - [ ] RSSI trilateration using path loss models from known anchors.
             - [ ] The system essentially operates in reverse: instead of using a known location and RSSI from multiple nodes to locate an arbitrary broadcast, we use a known broadcast location and RSSI to precisely locate our own nodes.
    - [ ] `BackgroundNoiseCalibration`: Measure noise floor (replaces Cluster Discovery).
        - [ ] Sweep all supported bands (315MHz, 433MHz, 868MHz, 915MHz, 928MHz).
        - [ ] Account for and advise about LNA presence, SWR issues, etc.
- [ ] **Operational Tasks**:
    - [ ] `WiFiBluetoothCensus`: Temporarily disconnects to scan all 2.4GHz devices (Wi-Fi/BT) for RSSI-based triangulation of all devices seen, then reconnects wifi after a specified delay.
- [ ] **Queue API**:
    - [x] `GET /api/queue`: List pending tasks.
    - [ ] `POST /api/queue`: Add new task.
    - [ ] `DELETE /api/queue/{id}`: Cancel/Remove task.
- [ ] **Preemption Logic**: Handling interrupt-priority commands vs background scanning.

## Phase 6: Telemetry & Peripheral Expansion
- [ ] **Geolocation Hardware Support**:
    - [ ] **GPS Module**: UART NMEA parsing (GY-NEO6MV2) for precision anchoring.
    - [ ] **Mesh GPS**: Active request for coordinates from attached node.
- [ ] **Hardware Probe**:
    - [ ] **Serial Probe**: Detect attached GPS (NMEA) or Meshtastic node on boot.
    - [ ] **I2C Scanner**: Auto-detect connected environment sensors (Temp, Air Quality, Geiger).
- [ ] **Meshtastic Integration**:
    - [ ] Serial API Client: Send/Receive messages via attached mesh node.
    - [ ] Configurable Channel: "Announce to Public" vs "Private Group".
    - [ ] **Chat Surveillance & Location**:
        - [ ] Monitor selected Public/Private channels.
        - [ ] **Single-Hop**: Geolocate sender via RSSI-trilateration.
        - [ ] **Multi-Hop**: Estimate distance/vector via Hop Count + Last-Hop neighbor RSSI-trilateration.
        - [ ] **Topology Mapping**: Use traceroutes to build a map of the mesh web, pinning nodes that report location or where we can locate them via RSSI-trilateration.
- [ ] **BLE Proximity & Consensus (Architecture)**:
    - [ ] **Dual-Role Operation**: ESP32 acts as both GAP Peripheral (Beacon) and GAP Central (Scanner).
        -   **Advertising**: Broadcasts `ClusterID` + `NodeID` + `TxPower` @ 1Hz.
        -   **Scanning**: Periodic windows to detect peers.
    - [ ] **Coexistence Strategy (Radio Time-Slicing)**:
        -   **Interval**: 1 scan window (e.g., 200ms) every 5-10 seconds to protect Wi-Fi throughput.
        -   **Adaptive Rate**: If RSSI variance < Threshold (Stationary) for 3 cycles, step down to 30s interval.
    - [ ] **Synchronized Sampling (Natural Consensus)**:
        -   Goal: Ensure all nodes measure each other at roughly the same moment for valid geometry.
        -   Mechanism: Use NTP time modulo (e.g., `UnixTime % 10 == 0`) to align scan windows across the cluster without complex negotiation.
- [ ] **Universal Telemetry Bus**:
    - [ ] Standardized polling for arbitrary sensors.
    - [ ] User-configurable announcement intervals.
    - [ ] Fallback Geolocation: Use attached GPS if available, else standard Wi-Fi/Radio fingerprinting.

## Phase 7: Distributed Coordination
- [ ] **Advanced Synchronization (VLBI)**:
    - [ ] **Microsecond Precision**: Track sub-second drift against GPS PPS if available.
    - [ ] **Multi-Transport Sync**: Support sync via Wi-Fi (NTP/PTP) and Meshtastic/LoRa (Custom Beacon).
    - [ ] **Latency Compensation**: Measure RTT (Round Trip Time) to compensate for transmission delay/light-speed lag across different mediums.
    - [ ] **Drift Correction**: Continuous adjustment for local clock drift relative to cluster consensus.
- [ ] **Sychronized Scanning**: Cluster leader designates frequency sweep windows and satart times, based on precisely synchronized time.
- [ ] **TDOA/RSSI Triangulation**: Aggregating data from the cluster to locate the sources of many broadcasts seen simultaneously in a sweep.

## Phase 8: Mesh Parity (Transport Independence)
- [ ] **Transport Agnosticism**: API, Telemetry, and Cluster Control must function transparently over both Wi-Fi and Meshtastic.
- [ ] **Remote Node Equivalence**: A remote node on the mesh should appear in the local Cluster UI just like a Wi-Fi peer.
- [ ] **Bandwidth Optimization**: Protocol compression to support full C2 (Command & Control) over low-bandwidth LoRa links.

---

# Application Architecture: The VLBI Cluster

## 1. Identity & Discovery (Implemented)
To function as a swarm, every node needs a unique identity on the network.
- **Hostname**: `allseeingeye-XXXXXX`, where `XXXXXX` is the last 3 bytes of the MAC address.
- **Discovery Hierarchy**:
    1.  **mDNS**: Primary method. Multicast announcement of `_allseeingeye._tcp`.
    2.  **Viral/Passive**: When Node A talks to Node B, Node B checks Node A.
    3.  **Brute Force**: If a node has 0 peers, it sequentially scans the entire `.1` to `.254` subnet looking for friends.
- **API Endpoints**:
    -   `/api/peers`: Returns list of known neighbors and their cluster/status.
    -   `/api/ping?target={ip}`: Manual connectivity check.
    -   `/api/config`: Set `cluster` name to move devices between groups.
    -   `/api/status`: Returns system health + current implementation details.

## 2. The concept of "Clustering"
Nodes are not solitary actors; they are members of a **Cluster**.
- **Default State**: Out of the box, all nodes belong to the cluster named `"Default"`.
- **Behavior**: All nodes in a cluster share the same configuration.
- **Formation**: A user can change a node's Cluster Name via the Web UI (or API). That node logically separates and begins listening for peers with the matching Cluster Name.

## 3. The User Interface (Zero-Conf)
The Web UI is hosted on *every* node, but any node can visualize the *entire* cluster.

**Layout Structure:**
1.  **Navbar**:
    *   Branding ("All-Seeing Eye")
    *   Connection Status (Online/Offline)
    *   Cluster Status: `Current Cluster: Default ✨ Working On: SystemIdle ✨`
2.  **Main Content Area (Tabbed View)**:
    *   **Clusters**: Tree view of all clusters and nodes (Auto-Grouping).
    *   **Task View**: Current operation controls (Scanner/Monitor) and task status.
    *   **Logs**: Real-time scrolling system logs (RingBuffer backed).
3.  **Visualization Panel**: Persistent spectrum analysis canvas (bottom of screen).
4.  **Footer**: Telemetry (RAM, Uptime) & Version info.

**Tabs Detail:**
1.  **Cluster Tab (Functional)**:
    -   A Tree View of all announced clusters on the subnet.
    -   `> Default Cluster`
        -   `allseeingeye-a1b2c3` Independent Exploration [Ready]
        -   `allseeingeye-f453a4` Hardware Verification [Working]
    -   `> Scanner-Group-1`
        -   `allseeingeye-224466` RSSI Sweep [Working]

2.  **Node Drill-down**:
    -   Clicking a node opens that device's dashboard.
    -   **Tooltip**: Hovering over a node displays exact task details.

## 4. Task Awareness (New)
The cluster is now "Task Aware".
-   **Default Task**: "Independent Exploration" (System Idle).
-   **Visibility**: All nodes broadcast their current high-level task via `/api/status`. 
-   **Logic**:
    -   `SystemIdlePlugin` -> "Independent Exploration".
    -   `RadioTestPlugin` -> "Hardware Verification".
    -   Future plugins will define their own strings (e.g., "Spectrum Sweep 900-928MHz").
-   **UI Representation**: 
    -   Format: `Hostname | Task Name | [Status]`
    -   Example: `allseeingeye-f453a | RSSI Scanning | [Working]`

## 5. Configuration Fields
To support this, the global configuration structure must expand:
```cpp
struct Config {
    char wifiSSID[32];
    char wifiPass[64];
    // ... current fields ...
    
    // Identity
    char clusterName[32];       // Default: "Default"
    char nodeDescription[255];  // Default: "New Node" or Empty
};
```

## 6. Radio Task Queue Architecture

To support complex operations (scanning, replaying, jamming) while maintaining cluster synchronization, the system moves from a "State Machine" model to a "Scheduled Task" model.

### 6.1 The "Singleton Radio" Problem
The CC1101 radio is a shared resource that cannot do two things at once. It cannot "listen for commands" while "scanning spectrum." Therefore, access must be arbitrated by a **Scheduler**.

### 6.2 Task Object Structure
A `RadioTask` is a self-contained unit of work:
*   **UUID**: Unique identifier for API tracking.
*   **Type**: `CRITICAL` (Startup), `CLUSTER` (Sync), `USER` (API), `BACKGROUND` (Idle).
*   **Plugin**: The specific logic to run (e.g., `Plugin::SpectrumSweep`).
*   **Parameters**: Frequency, duration, target settings.

### 6.3 Lifecycle & Boot Stages
The node does not immediately start "working" on boot. It follows a strict initialization path:

1.  **Phase 1: POST (Power-On Self Test)**
    *   Hardware Init, Filesystem Mount, Config Load.
    *   **Peripheral Probe**: Scan I2C/UART for GPS or Meshtastic nodes.
2.  **Phase 2: Infrastructure**
    *   Wi-Fi Connect, mDNS Start, HTTP Server Start, **NTP Sync**.
    *   **Telemetry Init**: Register any other discovered sensors.
3.  **Phase 3: Startup Queue (Calibration)**
    *   The `Scheduler` is pre-loaded with critical tasks:
    *   1. `RadioHardwareTest`: Ensure SPI comms.
    *   2. `GeolocationScan`: Fingerprint environment.
    *   3. `BackgroundNoiseCalibration`: Measure local RF noise floor.
    *   *(Note: Cluster Discovery is strictly Wi-Fi or Meshtastic Serial; no CC1101 beaconing by default)*.
4.  **Phase 4: Operational Loop**
    *   The Scheduler processes `Queue[0]`.
    *   If Queue is empty -> Insert `IdleTask` (e.g., Background Scan).

### 6.4 API Integration
*   **Enqueue**: Agents/Users `POST` tasks to `/api/queue`.
*   **Preemption**: High-priority tasks (User overrides) push current tasks to `PAUSED` or `ABORTED` state.
*   **Visibility**: `/api/status` returns the current queue depth and next scheduled operation.
*   **Self-Correction**: All API errors must return a `usage` key with a valid JSON example to allow Agents to retry automatically.

## 7. Cluster Network Topology Visualization
The Web UI includes a "Network Map" canvas to visualize the complex relationships between nodes, especially as we introduce varied transport layers (Wi-Fi, Meshtastic, VPN).

### 7.1 The Graph Model
*   **Vertices (Nodes)**: Each node (local or remote) is a vertex.
*   **Edges (Connections)**: A connection represents a visibility path.
    *   *Wi-Fi (LAN)*: Typically a full mesh (everyone sees everyone).
    *   *Meshtastic (LoRa)*: Partial mesh; Node A might see B, but A might not see C directly (multi-hop).
    *   *VPN*: Long-distance links connecting disparate subnets.
    *   *Indirect*: Connections reported by a peer that the local node cannot see directly.

### 7.2 Link Types (Edge Coloring)
The visualization distinguishes different types of relationships between nodes using color coding:

*   **Blue Links (Peer Visibility)**: 
    *   Represent logical visibility (Node A lists Node B in its peer registry).
    *   Length is arbitrary (physics-based force layout).
    *   Derived from `GET /api/peers` data.

*   **Red Links (Ranging Data) [Planned]**:
    *   Will represent physical distance measurements (Time-of-Flight or RSSI-Trilateration).
    *   Length is constrained to scale (e.g., 100px = 1 meter).
    *   Will include a text label showing the raw distance (e.g., "4.2m").
