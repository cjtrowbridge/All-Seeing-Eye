#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <Preferences.h>

class Config {
public:
    static Config& instance();

    void begin();
    String getWifiSSID();
    String getWifiPass();
    void setWifi(String ssid, String pass);

private:
    Config();
    Preferences _prefs;
};

#endif
