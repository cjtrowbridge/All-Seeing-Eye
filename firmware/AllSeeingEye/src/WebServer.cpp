#include "WebServer.h"
#include <LittleFS.h>
#include "Kernel.h" // For status access
#include "Logger.h" // Add Logger
#include "RingBuffer.h" // Add RingBuffer
#include "PluginManager.h" // Add PluginManager
#include "AsyncJson.h" 
#include <ArduinoJson.h>

WebServerManager& WebServerManager::instance() {
    static WebServerManager _instance;
    return _instance;
}

WebServerManager::WebServerManager() : _server(80) {}

void WebServerManager::begin() {
    setupRoutes();
    _server.begin();
    Serial.println("[Web] Async Server Started on Port 80");
}

void WebServerManager::setupRoutes() {
    // --------------------------------------------------
    // 1. Specific API Endpoints (Register First)
    // --------------------------------------------------

    // API: Status
    _server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request){
        JsonDocument doc;
        doc["uptime"] = millis();
        doc["heap_free"] = ESP.getFreeHeap();
        doc["psram_free"] = ESP.getFreePsram();
        doc["flash_size"] = ESP.getFlashChipSize();
        
        doc["rb_capacity"] = RingBuffer::instance().capacity();
        doc["rb_usage"] = RingBuffer::instance().available();

        doc["plugin"] = PluginManager::instance().getActivePluginName();

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    // API: Configuration (GET)
    _server.on("/api/config", HTTP_GET, [](AsyncWebServerRequest *request){
        String response = Config::instance().getAllAsJson();
        request->send(200, "application/json", response);
    });

    // API: Configuration (POST)
    AsyncCallbackJsonWebHandler *handler = new AsyncCallbackJsonWebHandler("/api/config", [](AsyncWebServerRequest *request, JsonVariant &json) {
        String jsonStr;
        serializeJson(json, jsonStr);
        
        if (Config::instance().updateFromJson(jsonStr)) {
            Logger::instance().info("Config", "Settings updated via API");
            request->send(200, "application/json", "{\"status\":\"success\", \"message\":\"Config Updated. Reboot to apply network changes.\"}");
            // Optional: Config::instance().save(); // Preferences are auto-saved in close/put
        } else {
            Logger::instance().error("Config", "JSON parsing failed");
            request->send(400, "application/json", "{\"status\":\"error\", \"message\":\"Invalid JSON\"}");
        }
    });
    _server.addHandler(handler);

    // API: File System Index
    _server.on("/api/fs", HTTP_GET, [](AsyncWebServerRequest *request){
        JsonDocument doc;
        JsonArray files = doc.to<JsonArray>();

        File root = LittleFS.open("/");
        File file = root.openNextFile();
        while(file){
            JsonObject f = files.add<JsonObject>();
            f["name"] = String(file.name());
            f["size"] = file.size();
            file = root.openNextFile();
        }
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    // API: System Logs
    _server.on("/api/logs", HTTP_GET, [](AsyncWebServerRequest *request){
        JsonDocument doc;
        JsonArray logs = doc.to<JsonArray>();
        
        std::deque<String> logBuffer = Logger::instance().getLogs();
        
        // Return logs in insertion order
        for(const auto& line : logBuffer) {
            logs.add(line);
        }
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    // --------------------------------------------------
    // 2. Generic API Endpoint (Discovery)
    // --------------------------------------------------
    _server.on("/api", HTTP_GET, [](AsyncWebServerRequest *request){
        JsonDocument doc;
        JsonArray routes = doc.to<JsonArray>();

        JsonObject r1 = routes.add<JsonObject>();
        r1["path"] = "/api";
        r1["method"] = "GET";
        r1["desc"] = "List available API endpoints";

        JsonObject r2 = routes.add<JsonObject>();
        r2["path"] = "/api/status";
        r2["method"] = "GET";
        r2["desc"] = "System health stats (RAM, Uptime)";

        JsonObject r3 = routes.add<JsonObject>();
        r3["path"] = "/api/config";
        r3["method"] = "GET/POST";
        r3["desc"] = "Get or Set system configuration";

        JsonObject r4 = routes.add<JsonObject>();
        r4["path"] = "/api/fs";
        r4["method"] = "GET";
        r4["desc"] = "List files in LittleFS";

        JsonObject r5 = routes.add<JsonObject>();
        r5["path"] = "/api/logs";
        r5["method"] = "GET";
        r5["desc"] = "Get system log buffer";

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    // --------------------------------------------------
    // 3. Static Files (Catch-All fallthrough)
    // --------------------------------------------------
    _server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
}
