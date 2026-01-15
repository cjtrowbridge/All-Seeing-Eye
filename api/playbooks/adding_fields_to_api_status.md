# Playbook: Adding Fields to the /api/status Endpoint

*Status: Stable*

## Objective
The `/api/status` endpoint is the heartbeat of the All Seeing Eye. It is polled frequently by the WebUI and other agents to understand the state of the device. This playbook details how to safely add new information to this endpoint while maintaining performance and stability.

## Architecture Context
To minimize overhead on the single-threaded ESP32 web server, the `/api/status` response is **cached in memory**. Be aware that memory is limited and only put essential, fast-to-retrieve data here.
*   **Source File**: `firmware/AllSeeingEye/src/WebServer.cpp`
*   **Method**: `WebServerManager::getCachedStatus()`
*   **Library**: `ArduinoJson`

## Prerequisites
*   The data you want to expose must be accessible from a global context (e.g., Singleton `instance()`, static variable, or hardware read).
*   Avoid blocking operations (e.g., long I/O, heavy computation) inside the status generation logic.

## Step-by-Step Instructions

### 1. Identify the Data Source
Determine where the data "lives". It should ideally be available via a simple getter method.
*   *Correct*: `HAL::instance().getBatteryVoltage()` (Fast, memory read)
*   *Incorrect*: `HAL::instance().readBatteryVoltageAndWait()` (Slow, blocking)

### 2. Modify `WebServer.cpp`
Open `firmware/AllSeeingEye/src/WebServer.cpp` and locate the `WebServerManager::getCachedStatus()` function.

Add your new field to the `JsonDocument`.

#### For Simple Values (Numbers, Strings, Booleans):
```cpp
// ... existing code ...
doc["plugin"] = PluginManager::instance().getActivePluginName();

// [NEW] Adding Battery Level
doc["voltage"] = HAL::instance().getBatteryVoltage();
```

#### For Nested Objects:
```cpp
// ... existing code ...
// Create the object container
JsonObject network = doc.createNestedObject("network");

// Add fields to the nested object
network["ssid"] = Config::instance().getSSID();
network["rssi"] = WiFi.RSSI();
```

### 3. Handling Complex/Large Data (Delegation Pattern)
If you are adding a list or complex structure maintaing by another module (like Peers or Logs), do **not** write the iteration logic inside `WebServer.cpp`. Instead, delegate it.

**Pattern:**
1.  **In WebServer.cpp**: Create the array and pass it to the manager.
    ```cpp
    JsonArray sensors = doc.createNestedArray("sensors");
    SensorManager::instance().populateSensorData(sensors);
    ```

2.  **In the Manager Class (Header)**:
    ```cpp
    // SensorManager.h
    void populateSensorData(JsonArray& arr);
    ```

3.  **In the Manager Class (Source)**:
    ```cpp
    // SensorManager.cpp
    void SensorManager::populateSensorData(JsonArray& arr) {
        for (auto& s : _sensors) {
            JsonObject obj = arr.createNestedObject();
            obj["id"] = s.id;
            obj["value"] = s.value;
        }
    }
    ```

### 4. Update Documentation
**CRITICAL**: You must document the new field in `/api/README.md`.

1.  Locate the **System Status** section in `/api/README.md`.
2.  Update the "Response" JSON example to include your new field.
3.  Add a description of the field if its meaning is not obvious.

### 5. Verification
1.  **Compile**: `arduino-cli compile ...`
2.  **Flash**: `upload_ota.ps1` or serial.
3.  **Check**:
    *   Command Line: `curl http://<device_ip>/api/status`
    *   Browser: Go to `http://<device_ip>/api/status`
    *   Verify the new field appears and has the correct value.

## Performance considerations
*   **JSON Size**: The `JsonDocument` has a fixed memory allocation. If you add too much data, `ArduinoJson` may run out of memory and fields will be missing silently. Monitor `doc.overflowed()` if you are adding large arrays.
*   **Cache Duration**: The cache is valid for a short time (e.g., 500ms - 1000ms). Ensure your data doesn't need to be fresher than that.
