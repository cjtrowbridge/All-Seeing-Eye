#include "PluginManager.h"
#include "Logger.h"

// Available Plugins
#include "SystemIdlePlugin.h"
#include "RadioTestPlugin.h"
#include "BleRangingPlugin.h"
#include "GeolocationPlugin.h"
#include "RfDiagPlugin.h"
#include "SpectrumPlugin.h"
#include "MeshtasticPlugin.h"
#include "TaskTypes.h"

PluginManager& PluginManager::instance() {
    static PluginManager _instance;
    return _instance;
}

PluginManager::PluginManager() : _activePlugin(nullptr) {
    _mutex = xSemaphoreCreateMutex();
}

void PluginManager::loadPlugin(ASEPlugin* newPlugin) {
    if (newPlugin == nullptr) return;

    // Block until we can safely switch (Core 1 not in middle of loop)
    xSemaphoreTake(_mutex, portMAX_DELAY);
    
    if (_activePlugin) {
        Logger::instance().info("PluginMgr", "Stopping plugin: %s...", _activePlugin->getName().c_str());
        _activePlugin->teardown();
        delete _activePlugin; // Clean up old memory
        _activePlugin = nullptr;
    }
    
    _activePlugin = newPlugin;
    Logger::instance().info("PluginMgr", "Starting plugin: %s...", _activePlugin->getName().c_str());
    _activePlugin->setup();
    
    xSemaphoreGive(_mutex);
}

// -------------------------------------------------------------------------
// Task Registry
// Define all available tasks here. 
// This mimics a database of capabilities.
// -------------------------------------------------------------------------
std::vector<TaskDefinition> PluginManager::getTaskCatalog() {
    std::vector<TaskDefinition> catalog;

    // 1. BLE Ranging
    catalog.push_back({
        "ble-ranging/peer", 
        "BLE Peer Ranging", 
        "BLE Ranging", 
        "Active scan + RSSI history logging for specific targets.",
        "/api/task/ble-ranging/peer"
    });
    catalog.push_back({
        "ble-ranging/survey", 
        "BLE Device Survey", 
        "BLE Ranging", 
        "Lists all nearby BLE MACs and payloads.",
        "/api/task/ble-ranging/survey"
    });

    // 2. Geolocation (Placeholder)
    catalog.push_back({
        "geolocation/fix",
        "Geolocation Fix",
        "Geolocation",
        "Aggregates GPS + WiFi anchors to determine location.",
        "/api/task/geolocation/fix"
    });

    // 3. System
    catalog.push_back({
        "system/idle",
        "System Idle",
        "System",
        "Low power background monitoring.",
        "/api/task/system/idle"
    });

    // 4. RF Diagnostics
    catalog.push_back({
        "rf-diag/noise",
        "Noise Floor Check",
        "RF Diagnostics",
        "Measures RSSI without sync word/packet logic.",
        "/api/task/rf-diag/noise"
    });

    // 5. Spectrum Analysis
    catalog.push_back({
       "spectrum/scan",
       "Band Scan",
       "Spectrum Analyzer",
       "Standard sweep, returns power levels.",
       "/api/task/spectrum/scan"
    });

    // 6. Meshtastic
    catalog.push_back({
        "meshtastic/monitor",
        "Traffic Monitor",
        "Meshtastic",
        "Logs all seen packets with RSSI/SNR metrics.",
        "/api/task/meshtastic/monitor"
    });
    catalog.push_back({
        "meshtastic/trace",
        "Network Traceroute",
        "Meshtastic",
        "Performs an active traceroute to map the hop path.",
        "/api/task/meshtastic/trace"
    });

    return catalog;
}

bool PluginManager::startTask(String taskId, JsonObject params) {
    Logger::instance().info("PluginMgr", "Requesting Task: %s", taskId.c_str());
    
    String pluginName = "";
    
    // Router Logic
    if (taskId.startsWith("ble-ranging")) {
        pluginName = "BleRanging";
    } else if (taskId.startsWith("system/idle")) {
        pluginName = "SystemIdle";
    } else if (taskId.startsWith("geolocation")) {
        pluginName = "Geolocation";
    } else if (taskId.startsWith("rf-diag")) {
        pluginName = "RfDiag";
    } else if (taskId.startsWith("spectrum")) {
        pluginName = "Spectrum";
    } else if (taskId.startsWith("meshtastic")) {
        pluginName = "Meshtastic";
    }
    
    if (pluginName == "") {
        Logger::instance().error("PluginMgr", "No plugin mapping for task: %s", taskId.c_str());
        return false;
    }

    // Create the new plugin
    ASEPlugin* plugin = createPlugin(pluginName);
    if (!plugin) return false;

    // Configure it BEFORE loading it (while it's just a pointer on Core 0 stack)
    plugin->configure(taskId, params);

    // Load it (Swaps active plugin safely)
    loadPlugin(plugin);
    
    return true;
}

ASEPlugin* PluginManager::createPlugin(String name) {
    if (name == "SystemIdle" || name == "Idle") {
        return new SystemIdlePlugin();
    }
    if (name == "RadioTest") {
        return new RadioTestPlugin();
    }
    if (name == "BleRanging") {
        return new BleRangingPlugin();
    }
    if (name == "Geolocation") return new GeolocationPlugin();
    if (name == "RfDiag") return new RfDiagPlugin();
    if (name == "Spectrum") return new SpectrumPlugin();
    if (name == "Meshtastic") return new MeshtasticPlugin();
    
    Logger::instance().error("PluginMgr", "Unknown plugin request: %s. Defaulting to Idle.", name.c_str());
    return new SystemIdlePlugin();
}

void PluginManager::runLoop() {
    // Try to take the mutex. If Core 0 is switching plugins, we wait.
    // If we fail to take it (unlikely with portMAX_DELAY), we skip.
    // Using a timeout allows the watchdog to notice if we deadlock.
    if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        if (_activePlugin) {
            _activePlugin->loop();
            // Optional: nice to others
            delay(1);
        } else {
            // No plugin loaded, just chill
            delay(100);
        }
        xSemaphoreGive(_mutex);
    } else {
        // Could be switching, yield
        delay(1);
    }
}

ASEPlugin* PluginManager::getActivePlugin() {
    return _activePlugin;
}

String PluginManager::getActivePluginName() {
    if (_activePlugin) {
        return _activePlugin->getName();
    }
    return "None";
}

String PluginManager::getActiveTaskName() {
    String t = "None";
    if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        if (_activePlugin) {
            t = _activePlugin->getTaskName();
        }
        xSemaphoreGive(_mutex);
    }
    return t;
}
