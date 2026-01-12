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

## Phase 3: Web Dashboard (Near Completion)
- [x] Basic SPA structure
- [x] WebSocket/Poll-based logs
- [x] **UI Refactor**: Moves stats to footer, fix overlap (Tabbed View).
- [x] **Navbar Enhancements**: Cluster Status with "Sparkle" indicators.
- [ ] **Spectrum Visualization**: Connect real RSSI data to canvas.
- [ ] **Config UI**: Connect forms to backend Config API.

## Phase 4: VLBI Clustering (Backend Completed)
- [x] **Core Logic**: Default Kernel state is now `SystemIdle` (POST -> Idle).
- [x] **Unique Identity**:
    -   Hostnames: `allseeingeye-{hexid}.local` (from MAC address).
- [x] **Discovery Protocol**:
    -   **Zero-Conf**: mDNS (`_allseeingeye._tcp`) auto-discovery.
    -   **Subnet Scanning**: Automatically scans local /24 subnet (1-254) if isolated.
    -   **Passive Discovery**: Intercepts requests to `/api/peers` to find new neighbors.
    -   **Negative Caching**: Ignores non-peer IPs for 12h to reduce network noise.
- [x] **Cluster Management**:
    -   Nodes advertise `cluster` text record in mDNS.
    -   Clusters visualized in Web UI Tree View.
    -   Configurable **Cluster Name** and **Ignore Hours**.

## Phase 5: Distributed Tasks
- [ ] **Time Synchronization**: NTP + microsecond-offset tracking for VLBI.
- [ ] **Sychronized Scanning**: Cluster leader designates frequency sweep windows.
- [ ] **TDOA/RSSI Triangulation**: Aggregating data from the cluster to locate Tx sources.

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
