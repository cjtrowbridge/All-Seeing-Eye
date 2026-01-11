#include "src/Kernel.h"

// Note: OTA Logic is temporarily removed during the restructure. 
// We will reintegrate it into the Kernel/WebServer in Phase 2/3.
// For now, upload via USB.

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
