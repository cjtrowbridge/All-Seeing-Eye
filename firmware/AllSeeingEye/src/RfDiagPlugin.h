#ifndef RFDIAGPLUGIN_H
#define RFDIAGPLUGIN_H

#include "ASEPlugin.h"
#include "Logger.h"

class RfDiagPlugin : public ASEPlugin {
public:
    void setup() override {
        Logger::instance().info("RfDiag", "Setup: Radio Diagnostics");
    }
    
    void loop() override {
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    
    void teardown() override {
        Logger::instance().info("RfDiag", "Teardown");
    }

    String getName() override { return "RfDiag"; }
    String getTaskName() override { return _taskName; }

    void configure(String taskId, const JsonObject& params) override {
        _taskName = taskId;
        if (params.containsKey("freq")) {
            float f = params["freq"];
            Logger::instance().info("RfDiag", "Frequency set to %.2f", f);
        }
    }

private:
    String _taskName = "RfDiag";
};

#endif
