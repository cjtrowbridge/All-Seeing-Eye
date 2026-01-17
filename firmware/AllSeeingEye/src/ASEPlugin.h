#ifndef ASEPLUGIN_H
#define ASEPLUGIN_H

#include <Arduino.h>
#include <ArduinoJson.h>

class ASEPlugin {
public:
    virtual ~ASEPlugin() {}

    // Lifecycle
    virtual void setup() = 0;
    virtual void loop() = 0; // Running on Core 1
    virtual void teardown() = 0;

    // Metadata
    virtual String getName() = 0;
    virtual String getTaskName() { return "Unknown Task"; } // Default task name
    virtual String getDescription() { return ""; }
    virtual String getVersion() { return "1.0.0"; }

    // Task & Configuration
    virtual void configure(String taskId, const JsonObject& params) {}
    
    // API Interaction (Core 0 requests this, usually protected by mutex in Manager)
    // Returns true if data was written to report object
    virtual bool getJsonData(JsonObject report) { return false; } 
    
    // Command Handling
    virtual void handleCommand(String command, String value) {}
};

#endif
