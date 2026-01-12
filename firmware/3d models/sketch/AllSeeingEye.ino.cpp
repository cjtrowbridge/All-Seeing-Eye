#include <Arduino.h>
#line 1 "C:\\Users\\CJ\\Documents\\GitHub\\All-Seeing-Eye\\firmware\\AllSeeingEye\\AllSeeingEye.ino"
#include "src/Kernel.h"
#include <WiFi.h>
#include <HTTPClient.h>

// Note: OTA Logic is temporarily removed during the restructure. 
// We will reintegrate it into the Kernel/WebServer in Phase 2/3.
// For now, upload via USB.

#line 9 "C:\\Users\\CJ\\Documents\\GitHub\\All-Seeing-Eye\\firmware\\AllSeeingEye\\AllSeeingEye.ino"
void setup();
#line 18 "C:\\Users\\CJ\\Documents\\GitHub\\All-Seeing-Eye\\firmware\\AllSeeingEye\\AllSeeingEye.ino"
void loop();
#line 9 "C:\\Users\\CJ\\Documents\\GitHub\\All-Seeing-Eye\\firmware\\AllSeeingEye\\AllSeeingEye.ino"
void setup() {
    // Initialize Serial here for earliest possible logging
    Serial.begin(115200);
    delay(1000); 

    // Handover to Kernel
    Kernel::instance().setup();
}

void loop() {
    Kernel::instance().loop();
}

