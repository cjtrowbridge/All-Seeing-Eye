#line 1 "C:\\Users\\CJ\\Documents\\GitHub\\All-Seeing-Eye\\firmware\\AllSeeingEye\\src\\WebServer.h"
#ifndef WEBSERVER_AS_H
#define WEBSERVER_AS_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

class WebServerManager {
public:
    static WebServerManager& instance();
    void begin();
    
private:
    WebServerManager();
    AsyncWebServer _server;
    
    void setupRoutes();
};

#endif
