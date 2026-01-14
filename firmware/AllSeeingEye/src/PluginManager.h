#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include "ASEPlugin.h"
#include <vector>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

class PluginManager {
public:
    static PluginManager& instance();

    // Use to switch plugins from Core 0
    // NOTE: PluginManager TAKES OWNERSHIP of the pointer and will delete the OLD plugin.
    void loadPlugin(ASEPlugin* newPlugin);
    
    // Factory Method
    ASEPlugin* createPlugin(String name);

    // The main loop for Core 1
    void runLoop();

    // Accessors
    ASEPlugin* getActivePlugin();
    String getActivePluginName();
    String getActiveTaskName(); // New helper

private:
    PluginManager();
    
    ASEPlugin* _activePlugin;
    SemaphoreHandle_t _mutex;
};

#endif
