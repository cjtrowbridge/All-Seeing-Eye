#ifndef SYSTEMIDLEPLUGIN_H
#define SYSTEMIDLEPLUGIN_H

#include "ASEPlugin.h"
#include "Logger.h"

class SystemIdlePlugin : public ASEPlugin {
public:
    void setup() override {
        Logger::instance().info("Idle", "System entering Idle state.");
    }
    
    void loop() override {
        // Heartbeat roughly every 10 seconds
        static unsigned long lastTick = 0;
        if (millis() - lastTick > 10000) {
            lastTick = millis();
            Logger::instance().info("Idle", "zZz...");
        }
        // No delay here, return control to Manager
    }
    
    void teardown() override {
        Logger::instance().info("Idle", "Leaving Idle state.");
    }
    
    String getName() override {
        return "SystemIdle";
    }

    String getTaskName() override {
        return "Independent Exploration";
    }
};

#endif
