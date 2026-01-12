#line 1 "C:\\Users\\CJ\\Documents\\GitHub\\All-Seeing-Eye\\firmware\\AllSeeingEye\\src\\PluginManager.h"
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
    void loadPlugin(ASEPlugin* newPlugin);
    
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
