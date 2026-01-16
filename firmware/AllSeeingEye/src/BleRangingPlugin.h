#ifndef BLERANGINGPLUGIN_H
#define BLERANGINGPLUGIN_H

#include "ASEPlugin.h"
#include "BleRangingManager.h"
#include "Logger.h"

class BleRangingPlugin : public ASEPlugin {
public:
    void setup() override {
        Logger::instance().info("BleRanging", "BLE ranging task starting");
        BleRangingManager::instance().begin();
    }

    void loop() override {
        BleRangingManager::instance().loop();
        delay(5);
    }

    void teardown() override {
        Logger::instance().info("BleRanging", "BLE ranging task stopping");
        BleRangingManager::instance().stop();
    }

    String getName() override {
        return "BleRanging";
    }

    String getTaskName() override {
        return "BLE Ranging";
    }
};

#endif
