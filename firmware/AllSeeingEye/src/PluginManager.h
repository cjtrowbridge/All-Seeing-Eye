#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include "ASEPlugin.h"
#include "TaskTypes.h"
#include <vector>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

class PluginManager {
public:
    static PluginManager& instance();

    // Registry
    std::vector<TaskDefinition> getTaskCatalog(); 
    bool startTask(String taskId, JsonObject params); // Returns true if task started

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
