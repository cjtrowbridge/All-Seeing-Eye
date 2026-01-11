#include "Kernel.h"
#include "Logger.h"
#include <ArduinoOTA.h>
// Secrets are currently used for hardcoded WiFi fallback
#include "../secrets.h" 

Kernel& Kernel::instance() {
    static Kernel _instance;
    return _instance;
}

Kernel::Kernel() {}

void Kernel::setup() {
    // 1. Initialize HAL (LEDs, hardware)
    HAL::instance().init();
    HAL::instance().setLed(128, 100, 0); // Orange (Booting)

    Logger::instance().info("Kernel", "====== ASE-OS Kernel Booting ======");

    // 2. Hardware POST
    if (!HAL::instance().checkRadio()) {
        HAL::instance().setLed(255, 0, 0); // Red (Failure)
        Logger::instance().error("Kernel", "Critical Hardware Failure!");
        while(1) delay(1000);
    }

    // 3. File System
    setupLittleFS();

    // 4. Config
    Config::instance().begin();

    // 5. Network & WiFi
    setupWiFi();
    setupOTA();

    // 6. Web Server
    WebServerManager::instance().begin();

    // 7. Start Plugin Task (Core 1)
    xTaskCreatePinnedToCore(
        pluginTask,   // Function
        "PluginTask", // Name
        10000,        // Stack size
        NULL,         // Params
        1,            // Priority
        NULL,         // Handle
        1             // Core 1
    );

    HAL::instance().setLed(128, 0, 128); // Purple (Ready)
    Logger::instance().info("Kernel", "System Ready. All-Seeing Eye is open.");
}

void Kernel::loop() {
    // Core 0 Maintenance Loop
    ArduinoOTA.handle();

    // WiFi handling, OTA, etc implicitly handled by events
    // We can add watchdog or status logging here
    static unsigned long lastLog = 0;
    if (millis() - lastLog > 5000) {
        lastLog = millis();
        // Use Logger instead of Serial
        Logger::instance().info("Kernel", "Uptime: %lu ms | Heap: %d bytes | PSRAM: %d bytes", 
            millis(), ESP.getFreeHeap(), ESP.getFreePsram());
    }
}

void Kernel::setupLittleFS() {
    if (!LittleFS.begin(true)) { // true = formatOnFail
        Logger::instance().error("Kernel", "LittleFS Mount Failed");
        return;
    }
    Logger::instance().info("Kernel", "LittleFS Mounted Successfully");
}

void Kernel::setupWiFi() {
    Logger::instance().info("Kernel", "Connecting to WiFi...");
    // TODO: Load from Config::instance().getWifiSSID() in Phase 2
    // For now use secrets.h
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    // Simple blocking connect for prototype
    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 20) {
        delay(500);
        // We generally avoid logging every dot in our memory buffer, so we keep Serial for dots
        Serial.print("."); 
        retries++;
    }
    Serial.println("");
    
    if (WiFi.status() == WL_CONNECTED) {
        Logger::instance().info("Kernel", "WiFi Connected. IP: %s", WiFi.localIP().toString().c_str());
    } else {
        Logger::instance().warn("Kernel", "WiFi Connection Failed (using default credentials)");
    }
}

void Kernel::setupOTA() {
    ArduinoOTA.setHostname("AllSeeingEye");
    
    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else { // U_SPIFFS
            type = "filesystem";
        }
        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        LittleFS.end();
        Logger::instance().info("OTA", "Start updating %s", type.c_str());
    });
    
    ArduinoOTA.onEnd([]() {
        Logger::instance().info("OTA", "End");
    });
    
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        // Avoid flooding logs, maybe just serial dots or nothing
    });
    
    ArduinoOTA.onError([](ota_error_t error) {
        const char* err = "Unknown";
        if (error == OTA_AUTH_ERROR) err = "Auth Failed";
        else if (error == OTA_BEGIN_ERROR) err = "Begin Failed";
        else if (error == OTA_CONNECT_ERROR) err = "Connect Failed";
        else if (error == OTA_RECEIVE_ERROR) err = "Receive Failed";
        else if (error == OTA_END_ERROR) err = "End Failed";
        Logger::instance().error("OTA", "Error[%u]: %s", error, err);
    });

    ArduinoOTA.begin();
    Logger::instance().info("Kernel", "OTA Service Started");
}

void Kernel::pluginTask(void* parameter) {
    Logger::instance().info("Core1", "Plugin Engine Started");
    while(true) {
        // Phase 3: Execute ActivePlugin->loop()
        // For now, just yield
        delay(10);
    }
}
