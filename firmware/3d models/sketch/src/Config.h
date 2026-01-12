#line 1 "C:\\Users\\CJ\\Documents\\GitHub\\All-Seeing-Eye\\firmware\\AllSeeingEye\\src\\Config.h"
#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <Preferences.h>
#include <ArduinoJson.h>

class Config {
public:
    static Config& instance();

    void begin();

    // WiFi Accessors
    String getWifiSSID();
    String getWifiPass();
    void setWifi(String ssid, String pass);

    // System Settings
    String getHostname();
    void setHostname(String hostname);

    // Generic Accessors
    String getString(const char* key, String defaultValue);
    void setString(const char* key, String value);
    int getInt(const char* key, int defaultValue);
    void setInt(const char* key, int value);

    // Serialization
    String getAllAsJson(); 
    bool updateFromJson(String jsonBody);

private:
    Config();
    Preferences _prefs;
};

#endif
