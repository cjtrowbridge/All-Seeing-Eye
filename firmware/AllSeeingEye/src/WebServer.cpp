#include "WebServer.h"
#include <LittleFS.h>
#include "Kernel.h" // For status access
#include "Logger.h" // Add Logger

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
    // 1. Static Files (Dashboard)
    // Serves files from LittleFS root
    _server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

    // 2. API: Status
    _server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request){
        JsonDocument doc;
        doc["uptime"] = millis();
        doc["heap_free"] = ESP.getFreeHeap();
        doc["psram_free"] = ESP.getFreePsram();
        doc["flash_size"] = ESP.getFlashChipSize();
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    // 3. API: File System Index
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

    // 4. API: System Logs
    _server.on("/api/logs", HTTP_GET, [](AsyncWebServerRequest *request){
        JsonDocument doc;
        JsonArray logs = doc.to<JsonArray>();
        
        std::deque<String> logBuffer = Logger::instance().getLogs();
        
        // Return logs in reverse order (newest first) usually preferred for UI, 
        // or standard order. Let's do standard order.
        for(const auto& line : logBuffer) {
            logs.add(line);
        }
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });
}
