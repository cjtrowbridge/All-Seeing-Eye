#include "Config.h"
// #include "../secrets.example.h" // Removed to prevent linker collision
// Note: We use the secrets.h at runtime if available via other means, 
// but here we just implementation Preferences for persistent storage.
// Initial defaults might come from secrets.

Config& Config::instance() {
    static Config _instance;
    return _instance;
}

Config::Config() {}

void Config::begin() {
    _prefs.begin("ase-config", false);
}

String Config::getWifiSSID() {
    // If not set in NVS, return generic default or handle initial setup
    return _prefs.getString("ssid", "");
    // Note: For the prototype, we still rely on secrets.h in Kernel logic 
    // until we have a full Provisioning mode. 
}

String Config::getWifiPass() {
    return _prefs.getString("pass", "");
}

void Config::setWifi(String ssid, String pass) {
    _prefs.putString("ssid", ssid);
    _prefs.putString("pass", pass);
}

String Config::getHostname() {
    return _prefs.getString("hostname", "AllSeeingEye");
}

void Config::setHostname(String hostname) {
    _prefs.putString("hostname", hostname);
}

// Generic wrappers
String Config::getString(const char* key, String defaultValue) {
    return _prefs.getString(key, defaultValue);
}

void Config::setString(const char* key, String value) {
    _prefs.putString(key, value);
}

int Config::getInt(const char* key, int defaultValue) {
    return _prefs.getInt(key, defaultValue);
}

void Config::setInt(const char* key, int value) {
    _prefs.putInt(key, value);
}

// Serialization
String Config::getAllAsJson() {
    JsonDocument doc;
    
    // Explicitly list exported keys to control visibility
    doc["ssid"] = getWifiSSID();
    doc["hostname"] = getHostname();
    // We do NOT export the password for security, or we mask it
    doc["pass"] = "******"; 
    
    // Add other future settings here (Plugin defaults, etc)
    // doc["defaultPlugin"] = getString("defPlugin", "RSSIScanner");

    String output;
    serializeJson(doc, output);
    return output;
}

bool Config::updateFromJson(String jsonBody) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonBody);

    if (error) {
        return false;
    }

    // Update WiFi if present
    if (doc.containsKey("ssid")) {
        setString("ssid", doc["ssid"].as<String>());
    }
    if (doc.containsKey("pass")) {
        String p = doc["pass"].as<String>();
        // Only update if it's not the mask
        if (p != "******") {
            setString("pass", p);
        }
    }
    
    // Update Hostname
    if (doc.containsKey("hostname")) {
        setString("hostname", doc["hostname"].as<String>());
    }

    return true;
}
