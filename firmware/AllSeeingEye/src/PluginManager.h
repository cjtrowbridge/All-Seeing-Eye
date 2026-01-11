#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include "ASEPlugin.h"
#include <vector>

class PluginManager {
public:
    static PluginManager& instance();

    // Use to switch plugins from Core 0
    void loadPlugin(ASEPlugin* newPlugin);
    
    // The main loop for Core 1
    void runLoop();

    // Accessors
    ASEPlugin* getActivePlugin();
    String getActivePluginName();

private:
    PluginManager();
    
    ASEPlugin* _activePlugin;
    SemaphoreHandle_t _mutex;
};

#endif
