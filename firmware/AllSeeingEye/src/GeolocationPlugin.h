#ifndef GEOLOCATIONPLUGIN_H
#define GEOLOCATIONPLUGIN_H

#include "ASEPlugin.h"
#include "Logger.h"

class GeolocationPlugin : public ASEPlugin {
public:
    void setup() override {
        Logger::instance().info("Geolocation", "Setup: Acquiring Fix...");
    }
    
    void loop() override {
        // Mock Implementation
        // In real life: Poll GPS, Scan WiFi
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
    void teardown() override {
        Logger::instance().info("Geolocation", "Teardown");
    }

    String getName() override { return "Geolocation"; }
    String getTaskName() override { return _taskName; }

    void configure(String taskId, const JsonObject& params) override {
        _taskName = taskId;
        if (params.containsKey("timeout")) {
            long t = params["timeout"];
            Logger::instance().info("Geolocation", "Timeout set to %d", t);
        }
    }

private:
    String _taskName = "Geolocation";
};

#endif
