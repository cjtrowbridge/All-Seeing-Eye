#ifndef RADIOTESTPLUGIN_H
#define RADIOTESTPLUGIN_H

#include "ASEPlugin.h"
#include "HAL.h"
#include "Logger.h"

class RadioTestPlugin : public ASEPlugin {
public:
    void setup() override {
        Logger::instance().info("RadioTest", "Setting up Radio on 915.0 MHz...");
        CC1101* radio = HAL::instance().getRadio();
        
        if (radio == nullptr) {
            Logger::instance().error("RadioTest", "Radio not initialized!");
            return;
        }

        // Basic config
        int state = radio->setFrequency(915.0);
        if (state == RADIOLIB_ERR_NONE) {
             Logger::instance().info("RadioTest", "Freq set to 915.0 MHz");
        } else {
             Logger::instance().error("RadioTest", "Set Freq Failed: %d", state);
        }

        radio->setOutputPower(10); // dBm
    }

    void loop() override {
        CC1101* radio = HAL::instance().getRadio();
        if (!radio) {
            delay(1000);
            return;
        }

        // Just read RSSI every second
        static unsigned long lastCheck = 0;
        if (millis() - lastCheck > 1000) {
            lastCheck = millis();
            float rssi = radio->getRSSI();
            Logger::instance().info("RadioTest", "RSSI: %f dBm", rssi);
            
            // Blink LED based on signal?
            // -100 is silence, -30 is strong
            int r = map((long)rssi, -120, -30, 0, 255);
            r = constrain(r, 0, 255);
            HAL::instance().setLed(r, 0, 0); // Red intensity
        }
        
        // Don't hog the core
        delay(10); 
    }

    void teardown() override {
        HAL::instance().setLed(0,0,0);
        Logger::instance().info("RadioTest", "Stopped.");
    }

    String getName() override {
        return "RadioTest";
    }
};

#endif
