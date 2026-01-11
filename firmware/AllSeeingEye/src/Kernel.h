#ifndef KERNEL_H
#define KERNEL_H

#include <Arduino.h>
#include "HAL.h"
#include "Config.h"
#include "WebServer.h"
#include <LittleFS.h>
#include <WiFi.h>

class Kernel {
public:
    static Kernel& instance();
    
    void setup();
    void loop();

private:
    Kernel();
    
    void setupLittleFS();
    void setupWiFi();
    void setupOTA();
    
    // Core 1 Task
    static void pluginTask(void* parameter);
};

#endif
