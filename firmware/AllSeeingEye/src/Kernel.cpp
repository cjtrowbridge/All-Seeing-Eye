#include "Kernel.h"
#include <WiFi.h>
#include <ArduinoOTA.h>
#include "LittleFS.h"
#include "Config.h"
#include "Logger.h"
#include "HAL.h"
#include "PeerManager.h"
#include <ESPmDNS.h>
#include "RingBuffer.h"
#include "PluginManager.h"
#include "Scheduler.h"
#include <time.h>
#include <esp_sntp.h>
#include "Geolocation.h"
#include "BleRangingManager.h"

// Secrets are currently used for hardcoded WiFi fallback
#include "../secrets.h" 

namespace {
struct TimezoneMapping {
    const char* iana;
    const char* posix;
};

const TimezoneMapping kTimezoneMappings[] = {
    {"America/Los_Angeles", "PST8PDT,M3.2.0,M11.1.0"},
    {"America/Denver", "MST7MDT,M3.2.0,M11.1.0"},
    {"America/Chicago", "CST6CDT,M3.2.0,M11.1.0"},
    {"America/New_York", "EST5EDT,M3.2.0,M11.1.0"},
    {"America/Phoenix", "MST7"},
    {"America/Anchorage", "AKST9AKDT,M3.2.0,M11.1.0"},
    {"Pacific/Honolulu", "HST10"},
    {"UTC", "UTC0"}
};

String normalizeTimezone(const String& timezone) {
    for (size_t i = 0; i < (sizeof(kTimezoneMappings) / sizeof(kTimezoneMappings[0])); ++i) {
        if (timezone.equalsIgnoreCase(kTimezoneMappings[i].iana)) {
            return String(kTimezoneMappings[i].posix);
        }
    }
    return timezone;
}
}

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
        Logger::instance().error("Kernel", "Critical Hardware Failure! Radio Missing.");
        _hardwareHealthy = false;
        // We continue anyway so the Web Interface is accessible to report the error
    } else {
        _hardwareHealthy = true;
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

    // 6.5 Time Sync (SNTP)
    setupTimeSync();

    // 7. Web Server
    WebServerManager::instance().begin();

    // 7.5 Peer Manager
    PeerManager::instance().begin();
    
    // 8. Task Scheduler
    Scheduler::instance().begin();

    // 8.5 Geolocation Core Service
    GeolocationService::instance().begin();

    // 8.6 BLE Ranging Manager (Core 0 service)
    // BleRangingManager::instance().begin(); // Moved to Plugin

    // 9. Start Plugin Task (Core 1)
    xTaskCreatePinnedToCore(
        pluginTask,   // Function
        "PluginTask", // Name
        10000,        // Stack size
        NULL,         // Params
        1,            // Priority
        NULL,         // Handle
        1             // Core 1
    );

   // 10. Startup Sequence
    if (_hardwareHealthy) {
        // Enqueue Hardware Verification
        RadioTask bootTest;
        bootTest.id = "BOOT-" + String(millis());
        bootTest.type = TASK_CRITICAL;
        bootTest.pluginName = "RadioTest";
        bootTest.taskName = "Hardware Verification";
        bootTest.durationMs = 5000; // Run for 5s then Idle
        
        Scheduler::instance().enqueue(bootTest);
        
        HAL::instance().setLed(128, 0, 128); // Purple (Ready)
    } else {
        // Just verify what we can (Idle)
        HAL::instance().setLed(255, 0, 0); // Keep Red (Error)
    }

    Logger::instance().info("Kernel", "System Ready. All-Seeing Eye is open.");
}

void Kernel::loop() {
    // Core 0 Maintenance Loop
    ArduinoOTA.handle();
    PeerManager::instance().loop(); // Handle Discovery
    Scheduler::instance().loop();   // Handle Tasks
    GeolocationService::instance().loop();
    // BleRangingManager::instance().loop(); // Moved to Plugin

    // Cluster Alignment: follow desired task if peers indicate one
    String clusterName = Config::instance().getString("cluster", "Default");
    String desiredTaskId;
    String desiredParamsJson;
    bool startRequested = false;
    unsigned long sourceProbeTime = 0;
    if (PeerManager::instance().getClusterDesiredTask(clusterName, desiredTaskId, desiredParamsJson, startRequested, sourceProbeTime)) {
        if (desiredTaskId.length() > 0 && (desiredTaskId != _desiredTaskId || desiredParamsJson != _desiredTaskParamsJson)) {
            _desiredTaskId = desiredTaskId;
            _desiredTaskParamsJson = desiredParamsJson;
            _startRequested = false;

            JsonDocument paramsDoc;
            if (_desiredTaskParamsJson.length() > 0) {
                deserializeJson(paramsDoc, _desiredTaskParamsJson);
            }
            JsonObject params = paramsDoc.as<JsonObject>();
            PluginManager::instance().deployTask(_desiredTaskId, params);
        }

        if (startRequested && !_startRequested) {
            _startRequested = true;
            PluginManager::instance().startStagedTask();
        }
    }

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

void Kernel::setDesiredTask(const String& taskId, const String& paramsJson) {
    _desiredTaskId = taskId;
    _desiredTaskParamsJson = paramsJson;
}

void Kernel::clearDesiredTask() {
    _desiredTaskId = "";
    _desiredTaskParamsJson = "";
    _startRequested = false;
}

String Kernel::getDesiredTaskId() {
    return _desiredTaskId;
}

String Kernel::getDesiredTaskParamsJson() {
    return _desiredTaskParamsJson;
}

void Kernel::setStartRequested(bool requested) {
    _startRequested = requested;
}

bool Kernel::isStartRequested() {
    return _startRequested;
}

void Kernel::setupLittleFS() {
    if (!LittleFS.begin(true)) { // true = formatOnFail
        Logger::instance().error("Kernel", "LittleFS Mount Failed");
        return;
    }
    Logger::instance().info("Kernel", "LittleFS Mounted Successfully");
}

void Kernel::setupWiFi() {
    String hostname = Config::instance().getHostname();
    Logger::instance().info("Kernel", "Setting Hostname: %s", hostname.c_str());
    WiFi.setHostname(hostname.c_str());

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

        // Setup mDNS
        if (MDNS.begin(hostname.c_str())) {
            Logger::instance().info("Kernel", "mDNS responder started: %s.local", hostname.c_str());
            MDNS.addService("allseeingeye", "tcp", 80);
            
            // Advertise Cluster Name
            String clusterName = Config::instance().getString("cluster", "Default");
            MDNS.addServiceTxt("allseeingeye", "tcp", "cluster", clusterName);
        } else {
            Logger::instance().error("Kernel", "Error setting up mDNS responder!");
        }

    } else {
        Logger::instance().error("Kernel", "WiFi Connection FAILED. Starting AP Mode...");
        WiFi.softAP("AllSeeingEye-Recovery");
        Logger::instance().info("Kernel", "AP Started: AllSeeingEye-Recovery");
    }
}

void Kernel::setupTimeSync() {
    String timezone = Config::instance().getTimezone();
    String tzApplied = normalizeTimezone(timezone);
    applyTimezone(tzApplied);

    Logger::instance().info("Kernel", "Timezone set: %s", timezone.c_str());
    if (tzApplied != timezone) {
        Logger::instance().info("Kernel", "Timezone POSIX: %s", tzApplied.c_str());
    }

    if (WiFi.status() != WL_CONNECTED) {
        Logger::instance().warn("Kernel", "Skipping SNTP init: WiFi not connected");
        return;
    }

    Logger::instance().info("Kernel", "Configuring SNTP: %s", tzApplied.c_str());
    configTzTime(tzApplied.c_str(), "pool.ntp.org", "time.nist.gov");
    sntp_set_sync_interval(60 * 60 * 1000);
}

void Kernel::applyTimezone(const String& timezone) {
    String tzApplied = normalizeTimezone(timezone);
    setenv("TZ", tzApplied.c_str(), 1);
    tzset();
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

bool Kernel::isTimeSynced() {
    time_t now = time(nullptr);
    if (now <= 0) {
        return false;
    }

    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    int year = timeinfo.tm_year + 1900;
    return year > 2025;
}

time_t Kernel::getEpochTime() {
    return time(nullptr);
}

String Kernel::getTimezone() {
    return Config::instance().getTimezone();
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
        
    // Cluster Name (Optional in future config)
    doc["clusterName"] = Config::instance().getString("cluster", "Default");

    String response;
    serializeJson(doc, response);
    Logger::instance().info("Kernel", "Status: %s", response.c_str());
}
