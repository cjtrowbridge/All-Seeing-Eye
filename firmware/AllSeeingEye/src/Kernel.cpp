#include "Kernel.h"
#include "Logger.h"
#include "RingBuffer.h" 
#include "PluginManager.h"
#include "SystemIdlePlugin.h"
#include <ArduinoOTA.h>
// Secrets are currently used for hardcoded WiFi fallback
#include "../secrets.h" 

// Pre-instantiate Plugins
SystemIdlePlugin idlePlugin;

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

    // 5. Ring Buffer (PSRAM)
    // Allocate 4MB for high-speed logging/data
    RingBuffer::instance().begin(4 * 1024 * 1024);

    // 6. Network & WiFi
    setupWiFi();
    setupOTA();

    // 7. Web Server
    WebServerManager::instance().begin();

    // 8. Start Plugin Task (Core 1)
    xTaskCreatePinnedToCore(
        pluginTask,   // Function
        "PluginTask", // Name
        10000,        // Stack size
        NULL,         // Params
        1,            // Priority
        NULL,         // Handle
        1             // Core 1
    );

    // 9. Load Default Plugin
    PluginManager::instance().loadPlugin(&idlePlugin);

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
    WiFi.mode(WIFI_STA);

    String confSSID = Config::instance().getWifiSSID();
    String confPass = Config::instance().getWifiPass();
    bool connected = false;

    // 1. Try Configured Credentials
    if (confSSID.length() > 0) {
        Logger::instance().info("Kernel", "Attempting connection to Saved SSID: %s", confSSID.c_str());
        WiFi.begin(confSSID.c_str(), confPass.c_str());
        
        int retries = 0;
        while (WiFi.status() != WL_CONNECTED && retries < 15) { // 7.5 seconds
            delay(500);
            Serial.print(".");
            retries++;
        }
        Serial.println("");
        
        if (WiFi.status() == WL_CONNECTED) {
            connected = true;
        } else {
            Logger::instance().warn("Kernel", "Saved credentials failed. Trying fallback...");
        }
    }

    // 2. Fallback to Secrets (if not connected)
    if (!connected) {
        Logger::instance().info("Kernel", "Attempting connection to Fallback SSID: %s", ssid);
        WiFi.begin(ssid, password);
        
        int retries = 0;
        while (WiFi.status() != WL_CONNECTED && retries < 20) {
            delay(500);
            Serial.print("."); 
            retries++;
        }
        Serial.println("");
        
        if (WiFi.status() == WL_CONNECTED) {
            connected = true;
        }
    }
    
    if (connected) {
        Logger::instance().info("Kernel", "WiFi Connected. IP: %s", WiFi.localIP().toString().c_str());
    } else {
        Logger::instance().error("Kernel", "WiFi Connection FAILED. Starting AP Mode...");
        WiFi.softAP("AllSeeingEye-Recovery");
        Logger::instance().info("Kernel", "AP Started: AllSeeingEye-Recovery");
    }
}

void Kernel::setupOTA() {
    String hostname = Config::instance().getHostname();
    ArduinoOTA.setHostname(hostname.c_str());
    
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
        PluginManager::instance().runLoop();
        // The runLoop handles its own yields if needed, 
        // but a small safety yield here doesn't hurt if runLoop exits early.
        delay(1); 
    }
}

void Kernel::getStatus(JsonObject& doc) {
    // Existing status fields
    doc["uptime"] = millis();
    doc["heap_free"] = ESP.getFreeHeap();
    doc["psram_free"] = ESP.getFreePsram();
    doc["flash_size"] = ESP.getFlashChipSize();

    // RingBuffer stats
    doc["rb_size"] = RingBuffer::instance().capacity();
    doc["rb_available"] = RingBuffer::instance().available();
        
    String response;
    serializeJson(doc, response);
    Logger::instance().info("Kernel", "Status: %s", response.c_str());
}
