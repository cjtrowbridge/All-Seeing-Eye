#line 1 "C:\\Users\\CJ\\Documents\\GitHub\\All-Seeing-Eye\\firmware\\AllSeeingEye\\src\\WebServer.cpp"
#include "WebServer.h"
#include <LittleFS.h>
#include "WebStatic.h" // Include generated HTML header
#include "Kernel.h" // For status access
#include "Logger.h" // Add Logger
#include "RingBuffer.h" // Add RingBuffer
#include "PluginManager.h" // Add PluginManager
#include "PeerManager.h" // Add PeerManager
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
        doc["heap_size"] = ESP.getHeapSize(); // Added for % calc
        doc["psram_free"] = ESP.getFreePsram();
        doc["psram_size"] = ESP.getPsramSize();
        doc["flash_size"] = ESP.getFlashChipSize();

        doc["hostname"] = Config::instance().getHostname();
        
        doc["rb_capacity"] = RingBuffer::instance().capacity();
        doc["rb_usage"] = RingBuffer::instance().available();

        doc["plugin"] = PluginManager::instance().getActivePluginName();

        // Calculate System Status
        String statusMsg = "Ready";
        if (!Kernel::instance().isHardwareHealthy()) {
            statusMsg = "Radio Problem: Failed To POST";
        } else {
            String pName = PluginManager::instance().getActivePluginName();
            if (pName == "SystemIdle") {
                statusMsg = "Ready";
            } else if (pName == "RadioTest") {
                statusMsg = "Working: Hardware Verification";
            } else {
                 statusMsg = "Working: " + pName;
            }
        }
        doc["status"] = statusMsg;
        doc["task"] = PluginManager::instance().getActiveTaskName();
        doc["clusterName"] = Config::instance().getString("cluster", "Default");

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

    // API: Peers (Discovery)
    _server.on("/api/peers", HTTP_GET, [](AsyncWebServerRequest *request){
        // Passive Discovery: Check who is calling us
        PeerManager::instance().trackIncomingRequest(request->client()->remoteIP().toString());
        
        String response = PeerManager::instance().getPeersAsJson();
        request->send(200, "application/json", response);
    });

    // API: Utils - Ping
    _server.on("/api/ping", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("target")) {
            String target = request->getParam("target")->value();
            bool result = PeerManager::instance().pingHost(target);
            
            JsonDocument doc;
            doc["target"] = target;
            doc["reachable"] = result;
            
            String res;
            serializeJson(doc, res);
            request->send(200, "application/json", res);
        } else {
            request->send(400, "application/json", "{\"error\":\"Missing 'target' parameter\"}");
        }
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

        JsonObject r6 = routes.add<JsonObject>();
        r6["path"] = "/api/peers";
        r6["method"] = "GET";
        r6["desc"] = "Get discovered peers list";

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    // --------------------------------------------------
    // 3. Static Files (Embedded & FS Fallback)
    // --------------------------------------------------

    // Serve Embedded index.html (Compressed)
    auto rootHandler = [](AsyncWebServerRequest *request){
        AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", index_html_gz, index_html_gz_len);
        response->addHeader("Content-Encoding", "gzip");
        request->send(response);
    };
    
    _server.on("/", HTTP_GET, rootHandler);
    _server.on("/index.html", HTTP_GET, rootHandler);

    // Fallback: Serve other static files from LittleFS (if we add images later)
    _server.serveStatic("/assets", LittleFS, "/assets");
}
