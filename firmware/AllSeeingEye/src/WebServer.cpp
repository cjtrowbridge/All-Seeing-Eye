#include "WebServer.h"
#include <LittleFS.h>
#include "WebStatic.h" // Include generated HTML header
#include "BuildVersion.h" // Include generated Build ID
#include "Kernel.h" // For status access
#include "Logger.h" // Add Logger
#include "RingBuffer.h" // Add RingBuffer
#include "PluginManager.h" // Add PluginManager
#include "PeerManager.h" // Add PeerManager
#include "AsyncJson.h" 
#include <ArduinoJson.h>
#include "Scheduler.h" // Add scheduler

WebServerManager& WebServerManager::instance() {
    static WebServerManager _instance;
    return _instance;
}

WebServerManager::WebServerManager() : _server(80) {}

void WebServerManager::begin() {
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    
    setupRoutes();
    _server.begin();
    Serial.println("[Web] Async Server Started on Port 80");
}

void WebServerManager::setupRoutes() {
    // --------------------------------------------------
    // 1. Specific API Endpoints (Register First)
    // --------------------------------------------------

    // API: Status
    _server.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest *request){
        static unsigned long lastStatusLog = 0;
        if (millis() - lastStatusLog > 10000) {
            Logger::instance().info("API", "GET /api/status");
            lastStatusLog = millis();
        }
        request->send(200, "application/json", this->getCachedStatus());
    });

    // API: Configuration (GET)
    _server.on("/api/config", HTTP_GET, [](AsyncWebServerRequest *request){
        Logger::instance().info("API", "GET /api/config");
        String response = Config::instance().getAllAsJson();
        request->send(200, "application/json", response);
    });

    // API: Configuration (POST)
    AsyncCallbackJsonWebHandler *handler = new AsyncCallbackJsonWebHandler("/api/config", [](AsyncWebServerRequest *request, JsonVariant &json) {
        Logger::instance().info("API", "POST /api/config");
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
        Logger::instance().info("API", "GET /api/fs");
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
        Logger::instance().info("API", "GET /api/peers");
        // Passive Discovery: Check who is calling us
        PeerManager::instance().trackIncomingRequest(request->client()->remoteIP().toString());
        
        String response = PeerManager::instance().getPeersAsJson();
        request->send(200, "application/json", response);
    });

    // API: Utils - Ping
    _server.on("/api/ping", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("target")) {
            String target = request->getParam("target")->value();
            Logger::instance().info("API", "GET /api/ping target=%s", target.c_str());
            bool result = PeerManager::instance().pingHost(target);
            
            JsonDocument doc;
            doc["target"] = target;
            doc["reachable"] = result;
            
            String res;
            serializeJson(doc, res);
            request->send(200, "application/json", res);
        } else {
            Logger::instance().warn("API", "GET /api/ping missing target");
            request->send(400, "application/json", "{\"error\":\"Missing 'target' parameter\"}");
        }
    });

    // API: System Logs
    _server.on("/api/logs", HTTP_GET, [](AsyncWebServerRequest *request){
        static unsigned long lastLogsLog = 0;
        if (millis() - lastLogsLog > 5000) {
            Logger::instance().info("API", "GET /api/logs");
            lastLogsLog = millis();
        }
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

    // API: System Head Logs (Startup Logs)
    _server.on("/api/logs/head", HTTP_GET, [](AsyncWebServerRequest *request){
        static unsigned long lastHeadLogsLog = 0;
        if (millis() - lastHeadLogsLog > 5000) {
            Logger::instance().info("API", "GET /api/logs/head");
            lastHeadLogsLog = millis();
        }
        JsonDocument doc;
        JsonArray logs = doc.to<JsonArray>();
        
        std::deque<String> logBuffer = Logger::instance().getHeadLogs();
        
        for(const auto& line : logBuffer) {
            logs.add(line);
        }
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    // --------------------------------------------------
    // API: LED Control
    // --------------------------------------------------
    
    // GET /api/led?r=255&g=0&b=0
    _server.on("/api/led", HTTP_GET, [](AsyncWebServerRequest *request){
        if(request->hasParam("r") && request->hasParam("g") && request->hasParam("b")) {
            int r = request->getParam("r")->value().toInt();
            int g = request->getParam("g")->value().toInt();
            int b = request->getParam("b")->value().toInt();
            Logger::instance().info("API", "GET /api/led r=%d g=%d b=%d", r, g, b);
            HAL::instance().setLed(r, g, b);
        } else {
            Logger::instance().warn("API", "GET /api/led missing rgb params");
        }
        
        // Return Status
        JsonDocument doc;
        doc["power"] = HAL::instance().getLedPower();
        JsonObject color = doc.createNestedObject("color");
        uint32_t c = HAL::instance().getLedColor();
        color["r"] = (c >> 16) & 0xFF;
        color["g"] = (c >> 8) & 0xFF;
        color["b"] = c & 0xFF;
        
        String res;
        serializeJson(doc, res);
        request->send(200, "application/json", res);
    });

    // POST /api/led/on
    _server.on("/api/led/on", HTTP_POST, [](AsyncWebServerRequest *request){
        Logger::instance().info("API", "POST /api/led/on");
        HAL::instance().setLedPower(true);
        request->send(200, "application/json", "{\"status\":\"success\", \"power\":true}");
    });

    // POST /api/led/off
    _server.on("/api/led/off", HTTP_POST, [](AsyncWebServerRequest *request){
        Logger::instance().info("API", "POST /api/led/off");
        HAL::instance().setLedPower(false);
        request->send(200, "application/json", "{\"status\":\"success\", \"power\":false}");
    });

    // --------------------------------------------------
    // API: Queue (Task Scheduler)
    // --------------------------------------------------
    _server.on("/api/queue", HTTP_GET, [](AsyncWebServerRequest *request){
        Logger::instance().info("API", "GET /api/queue");
        JsonDocument doc;
        
        // Current Task
        RadioTask current = Scheduler::instance().getCurrentTask();
        JsonObject currObj = doc.createNestedObject("current");
        currObj["id"] = current.id;
        currObj["name"] = current.taskName;
        currObj["plugin"] = current.pluginName;
        currObj["elapsed"] = millis() - current.startTime;
        currObj["duration"] = current.durationMs;
        
        // Queue
        JsonArray qArr = doc.createNestedArray("queue");
        std::deque<RadioTask> queue = Scheduler::instance().getQueue();
        for(const auto& t : queue) {
            JsonObject obj = qArr.add<JsonObject>();
            obj["id"] = t.id;
            obj["name"] = t.taskName;
            obj["plugin"] = t.pluginName;
            obj["type"] = (int)t.type;
        }

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    // --------------------------------------------------
    // 2. Generic API Endpoint (Discovery)
    // --------------------------------------------------
    _server.on("/api", HTTP_GET, [](AsyncWebServerRequest *request){
        Logger::instance().info("API", "GET /api");
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
        r5["desc"] = "Get system log buffer (Tail)";

        JsonObject r5a = routes.add<JsonObject>();
        r5a["path"] = "/api/logs/head";
        r5a["method"] = "GET";
        r5a["desc"] = "Get system startup logs (Head)";

        JsonObject r6 = routes.add<JsonObject>();
        r6["path"] = "/api/peers";
        r6["method"] = "GET";
        r6["desc"] = "Get discovered peers list";

        JsonObject rLed = routes.add<JsonObject>();
        rLed["path"] = "/api/led";
        rLed["method"] = "GET";
        rLed["desc"] = "Set LED usage: /api/led?r=255&g=0&b=0";

        JsonObject rPower = routes.add<JsonObject>();
        rPower["path"] = "/api/led/on";
        rPower["method"] = "POST";
        rPower["desc"] = "Enable LED output";

        JsonObject r7 = routes.add<JsonObject>();
        r7["path"] = "/api/queue";
        r7["method"] = "GET";
        r7["desc"] = "Inspect task scheduler state";

        JsonObject r8 = routes.add<JsonObject>();
        r8["path"] = "/api/reboot";
        r8["method"] = "POST";
        r8["desc"] = "Restart the device";

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

    // API: Reboot
    _server.on("/api/reboot", HTTP_POST, [](AsyncWebServerRequest *request){
        Logger::instance().info("API", "POST /api/reboot");
        request->send(200, "application/json", "{\"status\":\"success\", \"message\":\"Rebooting...\"}");
        // Delay slightly to let the response flush
        // We can't delay in async handler easily, but ESP.restart() is abrupt.
        // A timer or loop check in main might be better, but often this works enough to close socket.
        // For safety, we set a flag or just restart.
        
        // Quick & Dirty: Defer restart to main loop or just do it.
        // AsyncWebserver runs in a task. calling restart here *might* be okay but cleaner to defer.
        // Let's defer using a timer callback or just do it after short delay.
        
        xTaskCreate([](void*){ 
            vTaskDelay(pdMS_TO_TICKS(100)); // Allow response to send
            ESP.restart(); 
            vTaskDelete(NULL);
        }, "reboot", 2048, NULL, 5, NULL);
    });
}

String WebServerManager::getCachedStatus() {
    // Check Validity (Time based + existence)
    if (millis() - _lastCacheTime < _cacheDuration && _cachedStatus.length() > 0) {
        return _cachedStatus;
    }

    // Dynamic Allocation
    JsonDocument doc; 
    
    doc["uptime"] = millis();
    doc["heap_free"] = ESP.getFreeHeap();
    doc["heap_size"] = ESP.getHeapSize();
    doc["psram_free"] = ESP.getFreePsram();
    doc["psram_size"] = ESP.getPsramSize();
    doc["flash_size"] = ESP.getFlashChipSize();

    doc["hostname"] = Config::instance().getHostname();
    doc["description"] = Config::instance().getString("description", "");
    doc["build_id"] = BUILD_ID;
    doc["timezone"] = Kernel::instance().getTimezone();
    doc["time"] = (long long)Kernel::instance().getEpochTime();
    doc["ntp_sync"] = Kernel::instance().isTimeSynced();
    
    doc["rb_capacity"] = RingBuffer::instance().capacity();
    doc["rb_usage"] = RingBuffer::instance().available();

    doc["plugin"] = PluginManager::instance().getActivePluginName();

    // LED Status
    JsonObject led = doc.createNestedObject("led");
    led["power"] = HAL::instance().getLedPower();
    JsonObject color = led.createNestedObject("color");
    uint32_t c = HAL::instance().getLedColor();
    color["r"] = (c >> 16) & 0xFF;
    color["g"] = (c >> 8) & 0xFF;
    color["b"] = c & 0xFF;

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

    // Queue Status
    RadioTask current = Scheduler::instance().getCurrentTask();
    JsonObject queueObj = doc.createNestedObject("queue");
    JsonObject qCurr = queueObj.createNestedObject("current");
    qCurr["id"] = current.id;
    qCurr["plugin"] = current.pluginName;
    qCurr["task"] = current.taskName;
    qCurr["elapsed"] = millis() - current.startTime;
    qCurr["duration"] = current.durationMs;
    queueObj["depth"] = Scheduler::instance().getQueue().size();

    // Aggregated Data
    JsonArray peers = doc.createNestedArray("peers");
    PeerManager::instance().populatePeers(peers);

    JsonArray logs = doc.createNestedArray("logs");
    Logger::instance().populateLogs(logs);

    String response;
    serializeJson(doc, response);
    
    // Update Cache
    _cachedStatus = response;
    _lastCacheTime = millis();
    
    return response;
}
