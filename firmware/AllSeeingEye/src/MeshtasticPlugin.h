#ifndef MESHTASTICPLUGIN_H
#define MESHTASTICPLUGIN_H

#include "ASEPlugin.h"
#include "Logger.h"

class MeshtasticPlugin : public ASEPlugin {
public:
    void setup() override {
        Logger::instance().info("Meshtastic", "Setup: Initializing Serial Comms...");
    }
    
    void loop() override {
        // Listen to serial port
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    
    void teardown() override {
        Logger::instance().info("Meshtastic", "Teardown");
    }

    String getName() override { return "Meshtastic"; }
    String getTaskName() override { return _taskName; }

    void configure(String taskId, const JsonObject& params) override {
        _taskName = taskId;
    }

private:
    String _taskName = "Meshtastic";
};

#endif
