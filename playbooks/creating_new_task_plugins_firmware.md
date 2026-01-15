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

3.  **Register with Kernel**
    *   **File**: `firmware/AllSeeingEye/src/Kernel.cpp`.
    *   **Action**: In `bootstrapPlugins()` or `setup()`, verify how plugins are loaded. (Currently, the system might only support switching *to* a task if it's pre-registered or if the Kernel knows about it). *Note: Ensure you include the new header in Kernel.cpp*.

4.  **Add API Trigger**
    *   Ensure the `TaskManager` or `Kernel` can switch to this task via the `/api/task` endpoint using the `getName()` ID.

5.  **Compile and Deploy**
    *   Follow `how_to_deploy_firmware_updates.md`.

## Verification
1.  Deploy fw.
2.  POST `/api/task` with `{"id": "my_new_task"}`.
3.  Check `/api/status`. The `task` field should show "My New Task".
