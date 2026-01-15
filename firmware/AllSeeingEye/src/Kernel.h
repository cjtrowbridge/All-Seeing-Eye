#ifndef KERNEL_H
#define KERNEL_H

#include <Arduino.h>
#include "HAL.h"
#include "Config.h"
#include "WebServer.h"
#include <LittleFS.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <time.h>

class Kernel {
public:
    static Kernel& instance();
    
    void setup();
    void loop();
    
    // Status reporting
    void getStatus(JsonObject& doc);
    bool isHardwareHealthy() { return _hardwareHealthy; }
    bool isTimeSynced();
    time_t getEpochTime();
    String getTimezone();

private:
    Kernel();
    bool _hardwareHealthy = true;
    
    void setupLittleFS();
    void setupWiFi();
    void setupOTA();
    void setupTimeSync();
    void applyTimezone(const String& timezone);
    
    // Core 1 Task
    static void pluginTask(void* parameter);
};

#endif
