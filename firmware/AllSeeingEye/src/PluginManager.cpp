#include "PluginManager.h"
#include "Logger.h"

// Available Plugins
#include "SystemIdlePlugin.h"
#include "RadioTestPlugin.h"
#include "BleRangingPlugin.h"
#include "GeolocationPlugin.h"
#include "RfDiagPlugin.h"
#include "SpectrumPlugin.h"
#include "HAL.h"
#include "MeshtasticPlugin.h"
#include "TaskTypes.h"

PluginManager& PluginManager::instance() {
    static PluginManager _instance;
    return _instance;
}

PluginManager::PluginManager() : _activePlugin(nullptr), _taskRunning(false) {
    _mutex = xSemaphoreCreateMutex();
}

void PluginManager::loadPlugin(ASEPlugin* newPlugin, bool startRunning) {
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
    _taskRunning = startRunning;
    
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
    TaskInputDefinition spectrumStart;
    spectrumStart.name = "start";
    spectrumStart.label = "Start Frequency (MHz)";
    spectrumStart.type = "number";
    spectrumStart.required = true;
    spectrumStart.defaultType = INPUT_VALUE_NUMBER;
    spectrumStart.defaultNumber = HAL::kCc1101DefaultStartMhz;
    spectrumStart.hasStep = true;
    spectrumStart.step = 0.1f;
    spectrumStart.hasMin = true;
    spectrumStart.min = HAL::kCc1101Band1MinMhz;
    spectrumStart.hasMax = true;
    spectrumStart.max = HAL::kCc1101Band3MaxMhz;

    TaskInputDefinition spectrumStop;
    spectrumStop.name = "stop";
    spectrumStop.label = "Stop Frequency (MHz)";
    spectrumStop.type = "number";
    spectrumStop.required = true;
    spectrumStop.defaultType = INPUT_VALUE_NUMBER;
    spectrumStop.defaultNumber = HAL::kCc1101DefaultStopMhz;
    spectrumStop.hasStep = true;
    spectrumStop.step = 0.1f;
    spectrumStop.hasMin = true;
    spectrumStop.min = HAL::kCc1101Band1MinMhz;
    spectrumStop.hasMax = true;
    spectrumStop.max = HAL::kCc1101Band3MaxMhz;

    TaskInputDefinition spectrumBandwidth;
    spectrumBandwidth.name = "bandwidth";
    spectrumBandwidth.label = "Channel Bandwidth (kHz)";
    spectrumBandwidth.type = "number";
    spectrumBandwidth.required = true;
    spectrumBandwidth.defaultType = INPUT_VALUE_NUMBER;
    spectrumBandwidth.defaultNumber = HAL::kCc1101DefaultBandwidthKhz;
    spectrumBandwidth.hasStep = true;
    spectrumBandwidth.step = 1.0f;
    spectrumBandwidth.hasMin = true;
    spectrumBandwidth.min = HAL::kCc1101MinBandwidthKhz;
    spectrumBandwidth.hasMax = true;
    spectrumBandwidth.max = HAL::kCc1101MaxBandwidthKhz;

    TaskInputDefinition spectrumPower;
    spectrumPower.name = "power";
    spectrumPower.label = "Broadcast Power (dBm)";
    spectrumPower.type = "number";
    spectrumPower.required = true;
    spectrumPower.defaultType = INPUT_VALUE_NUMBER;
    spectrumPower.defaultNumber = HAL::kCc1101DefaultPowerDbm;
    spectrumPower.hasStep = true;
    spectrumPower.step = 1.0f;
    spectrumPower.hasMin = true;
    spectrumPower.min = HAL::kCc1101MinPowerDbm;
    spectrumPower.hasMax = true;
    spectrumPower.max = HAL::kCc1101MaxPowerDbm;

     catalog.push_back({
         "spectrum/scan",
         "Band Scan",
         "Spectrum Analyzer",
         "Standard sweep, returns power levels.",
         "/api/task/spectrum/scan",
         { spectrumStart, spectrumStop, spectrumBandwidth, spectrumPower }
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
    loadPlugin(plugin, true);
    
    return true;
}

bool PluginManager::deployTask(String taskId, JsonObject params) {
    Logger::instance().info("PluginMgr", "Deploying Task: %s", taskId.c_str());
    String pluginName = "";

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

    ASEPlugin* plugin = createPlugin(pluginName);
    if (!plugin) return false;

    plugin->configure(taskId, params);
    loadPlugin(plugin, false);
    return true;
}

bool PluginManager::startStagedTask() {
    if (!_activePlugin) return false;
    _taskRunning = true;
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
        if (_activePlugin && _taskRunning) {
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

bool PluginManager::isTaskRunning() {
    return _taskRunning;
}
