#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <NetworkUdp.h>
#include <ArduinoOTA.h>

// Board: ESP32-S3-WROOM-1 N16R8
// LED: GPIO 48 (Common for S3 DevKits) or 47.
#define RGB_LED_PIN 48 
#define NUM_PIXELS 1

// WIFI CONFIGURATION
#include "wifi.h"

Adafruit_NeoPixel pixels(NUM_PIXELS, RGB_LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
    // Initialize Serial for logging
    Serial.begin(115200);
    
    // Give time for USB to enumerate
    delay(2000); 

    Serial.println("====== All-Seeing-Eye Firmware v0.2 (OTA Enabled) ======");
    Serial.println("Initializing system...");

    // Initialize LED
    pixels.begin();
    pixels.setBrightness(50); // Scale down brightness

    // Set color to Yellow (Connecting...)
    pixels.setPixelColor(0, pixels.Color(128, 100, 0));
    pixels.show();

    // 1. Connect to Wi-Fi
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // 2. Setup OTA
    ArduinoOTA.setHostname("AllSeeingEye-S3");
    // ArduinoOTA.setPassword("admin"); // Optional

    ArduinoOTA
        .onStart([]() {
            String type;
            if (ArduinoOTA.getCommand() == U_FLASH)
                type = "sketch";
            else // U_SPIFFS
                type = "filesystem";
            Serial.println("Start updating " + type);
            // Turn LED Blue indicating update
            pixels.setPixelColor(0, pixels.Color(0, 0, 255));
            pixels.show();
        })
        .onEnd([]() {
            Serial.println("\nEnd");
            // Turn LED Green on success
            pixels.setPixelColor(0, pixels.Color(0, 255, 0));
            pixels.show();
        })
        .onProgress([](unsigned int progress, unsigned int total) {
            Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
        })
        .onError([](ota_error_t error) {
            Serial.printf("Error[%u]: ", error);
            if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
            else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
            else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
            else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
            else if (error == OTA_END_ERROR) Serial.println("End Failed");
            // Turn LED Red on failure
            pixels.setPixelColor(0, pixels.Color(255, 0, 0));
            pixels.show();
        });

    ArduinoOTA.begin();

    // Set color to Purple (Ready)
    Serial.println("OTA Ready.");
    pixels.setPixelColor(0, pixels.Color(128, 0, 128));
    pixels.show();
}

void loop() {
    ArduinoOTA.handle();
    
    // Non-blocking heartbeat
    static unsigned long lastMillis = 0;
    if (millis() - lastMillis > 5000) {
        lastMillis = millis();
        Serial.print("System Active (IP: ");
        Serial.print(WiFi.localIP());
        Serial.println(")");
    }
}
