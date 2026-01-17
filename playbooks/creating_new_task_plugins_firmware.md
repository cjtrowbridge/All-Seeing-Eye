# Playbook: Creating New Task Plugins (Firmware)

*Status: Draft*

## Objective
The All Seeing Eye firmware uses a Plugin architecture. Tasks like "Idle" or "Spectrum Sweep" are implemented as plugins. This playbook explains how to create a new one.

## Prerequisites
*   C++ Knowledge.
*   Access to `firmware/AllSeeingEye/src/`.

## Step-by-Step Instructions

1.  **Create the Plugin Header**
    *   **Location**: `firmware/AllSeeingEye/src/MyNewTaskPlugin.h`.
    *   **Inheritance**: Must inherit from `ASEPlugin`.
    *   **Required Methods**:
        *   `const char* getName()`: Returns unique ID (e.g., "my_new_task").
        *   `String getTaskName()`: Returns human-readable name (e.g., "My New Task").
        *   `void setup()`: Run once when task starts.
        *   `void loop()`: Run repeatedly.
        *   `void cleanUp()`: Run when task is stopped (free resources!).

2.  **Implement the Logic**
    *   Example:
    ```cpp
    class MyNewTaskPlugin : public ASEPlugin {
    public:
        // ... implementations ...
    };
    ```
    *   **Logging Requirement**: Emit logs for task start, key milestones, and failure paths using `Logger::instance()`.
    *   **Synchronization Rule (Spectrum)**: For spectrum sweeps, align sampling to wall-clock boundaries and only sweep when `utc_seconds % 10 == 0`.
    *   **CC1101 Safety Limits**: Enforce band, bandwidth, and power limits in the plugin (see `HAL` constants) to avoid unsafe configurations.

3.  **Register with Kernel**
    *   **File**: `firmware/AllSeeingEye/src/Kernel.cpp`.
    *   **Action**: 
        1. Open `firmware/AllSeeingEye/src/PluginManager.cpp`.
        2. Locate `getTaskCatalog()`.
        3. Add a new `catalog.push_back({...})` entry.
        4. **Naming Convention**: 
           - The 3rd parameter is the **Plugin Name**. Use a human-readable string with spaces (e.g., "Spectrum Analyzer", "System").
           - This string is used by the WebUI to group tasks in the catalog headers.
        5. Map the task ID to your class in `startTask()` and `createPlugin()`.
          6. **Inputs Schema**:
              - Define required inputs using `TaskInputDefinition` (name, label, type, defaults, min/max/step, required).
              - Inputs are emitted by `GET /api/task` and rendered automatically by the WebUI.
              - Use numeric defaults for number inputs and include bounds for validation.

4.  **Add API Trigger**
    *   Ensure the `TaskManager` or `Kernel` can switch to this task via the `/api/task` endpoint using the `getName()` ID.

5.  **Compile and Deploy**
    *   Follow `how_to_deploy_firmware_updates.md`.
    *   **WebUI Update**: If you changed metadata, the WebUI (which fetches `/api/task`) will automatically reflect the new grouping without frontend code changes.

## Verification
1.  Deploy fw.
2.  Open WebUI (Center Column > Task List).
3.  Verify your task appears under the correct Group Header.
4.  Click the task and verify the form loads (params).
5.  Launch and check `/api/status` for active state.
