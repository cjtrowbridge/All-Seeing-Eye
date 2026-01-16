#ifndef SPECTRUMPLUGIN_H
#define SPECTRUMPLUGIN_H

#include "ASEPlugin.h"
#include "Logger.h"

class SpectrumPlugin : public ASEPlugin {
public:
    void setup() override {
        Logger::instance().info("Spectrum", "Setup: Sweeping...");
    }
    
    void loop() override {
        vTaskDelay(pdMS_TO_TICKS(100)); // Fast loop for sweeping
    }
    
    void teardown() override {
        Logger::instance().info("Spectrum", "Teardown");
    }

    String getName() override { return "Spectrum"; }
    String getTaskName() override { return _taskName; }

    void configure(String taskId, const JsonObject& params) override {
        _taskName = taskId;
        if (params.containsKey("start") && params.containsKey("stop")) {
            Logger::instance().info("Spectrum", "Sweep %.2f-%.2f MHz", 
                params["start"].as<float>(), params["stop"].as<float>());
        }
    }

private:
    String _taskName = "Spectrum Analysis";
};

#endif
