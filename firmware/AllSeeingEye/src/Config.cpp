#include "Config.h"
#include <esp_mac.h> 

// #include "../secrets.example.h" // Removed to prevent linker collision
// Note: We use the secrets.h at runtime if available via other means, 
// but here we just implementation Preferences for persistent storage.
// Initial defaults might come from secrets.h.

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
    String stored = _prefs.getString("hostname", "");
    
    // If user has a custom hostname, use it.
    // If it's empty or the old default "AllSeeingEye", generate a unique one.
    if (stored.length() > 0 && stored != "AllSeeingEye") {
        return stored;
    }

    // Generate Unique Default: allseeingeye-a1b2c3
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    
    char hostname[32];
    snprintf(hostname, sizeof(hostname), "allseeingeye-%02x%02x%02x", mac[3], mac[4], mac[5]);
    
    return String(hostname);
}

void Config::setHostname(String hostname) {
    _prefs.putString("hostname", hostname);
}

String Config::getTimezone() {
    return _prefs.getString("timezone", "America/Los_Angeles");
}

void Config::setTimezone(String timezone) {
    _prefs.putString("timezone", timezone);
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
    
    // Cluster Config
    doc["cluster"] = getString("cluster", "Default");
    doc["description"] = getString("description", "");

    // Timezone
    doc["timezone"] = getTimezone();
    
    // Peer Discovery
    doc["peer_ignore_hours"] = getInt("peer_ignore_hours", 12);

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

    // Update Cluster
    if (doc.containsKey("cluster")) {
        setString("cluster", doc["cluster"].as<String>());
    }

    // Update Description
    if (doc.containsKey("description")) {
        setString("description", doc["description"].as<String>());
    }

    // Update Timezone
    if (doc.containsKey("timezone")) {
        setTimezone(doc["timezone"].as<String>());
    }

    // Peer Discovery
    if (doc.containsKey("peer_ignore_hours")) {
        setInt("peer_ignore_hours", doc["peer_ignore_hours"].as<int>());
    }

    return true;
}
