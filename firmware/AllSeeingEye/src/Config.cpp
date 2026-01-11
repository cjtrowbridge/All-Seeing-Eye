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
