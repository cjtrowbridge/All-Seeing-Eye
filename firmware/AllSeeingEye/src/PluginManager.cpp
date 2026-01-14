#include "PluginManager.h"
#include "Logger.h"

// Available Plugins
#include "SystemIdlePlugin.h"
#include "RadioTestPlugin.h"
// #include "ScannerPlugin.h" // Future

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

ASEPlugin* PluginManager::createPlugin(String name) {
    if (name == "SystemIdle" || name == "Idle") {
        return new SystemIdlePlugin();
    }
    if (name == "RadioTest") {
        return new RadioTestPlugin();
    }
    // Future: Scanner, Jammer, etc.
    
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
