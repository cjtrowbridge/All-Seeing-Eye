#include "PluginManager.h"
#include "Logger.h"

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
        // Note: We do not delete the object here, as we likely manage them as static or member instances in Kernel
    }
    
    _activePlugin = newPlugin;
    Logger::instance().info("PluginMgr", "Starting plugin: %s...", _activePlugin->getName().c_str());
    _activePlugin->setup();
    
    xSemaphoreGive(_mutex);
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
    // This might be called from Core 0, so we should really lock if we want to be 100% safe,
    // but reading a pointer to get a name string is generally low risk if the string itself is constant.
    // We'll take mutex for safety.
    String name = "None";
    // Wait up to 500ms for the loop to yield
    if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(500)) == pdTRUE) {
        if (_activePlugin) {
            name = _activePlugin->getName();
        }
        xSemaphoreGive(_mutex);
    }
    return name;
}
