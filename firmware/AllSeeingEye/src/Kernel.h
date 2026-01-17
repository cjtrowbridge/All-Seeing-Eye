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
    void applyTimezone(const String& timezone);

    // Cluster Task Coordination
    void setDesiredTask(const String& taskId, const String& paramsJson);
    void clearDesiredTask();
    String getDesiredTaskId();
    String getDesiredTaskParamsJson();
    void setStartRequested(bool requested);
    bool isStartRequested();

private:
    Kernel();
    bool _hardwareHealthy = true;
    String _desiredTaskId;
    String _desiredTaskParamsJson;
    bool _startRequested = false;
    
    void setupLittleFS();
    void setupWiFi();
    void setupOTA();
    void setupTimeSync();
    
    // Core 1 Task
    static void pluginTask(void* parameter);
};

#endif
