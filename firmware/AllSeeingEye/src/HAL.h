#ifndef HAL_H
#define HAL_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <RadioLib.h>
#include <SPI.h>

class HAL {
public:
    static HAL& instance();
    
    void init();
    
    // LED Control
    void setLed(uint8_t r, uint8_t g, uint8_t b); // Updates state and sets HW if Powered
    void setLedPower(bool on); // Master Switch
    
    // Status Accessor
    bool getLedPower();
    uint32_t getLedColor(); // Returns packed RGB
    
    // Hardware Capabilities
    bool hasRadio();      // CC1101
    bool hasGPS();        // Hardware GPS
    bool hasMeshtastic(); // LoRa Radio

    bool checkRadio(); // Power-On Self Test
    
    // Radio Access
    CC1101* getRadio();

    // CC1101 Safety Limits (Datasheet)
    static constexpr float kCc1101Band1MinMhz = 300.0f;
    static constexpr float kCc1101Band1MaxMhz = 348.0f;
    static constexpr float kCc1101Band2MinMhz = 387.0f;
    static constexpr float kCc1101Band2MaxMhz = 464.0f;
    static constexpr float kCc1101Band3MinMhz = 779.0f;
    static constexpr float kCc1101Band3MaxMhz = 928.0f;
    static constexpr float kCc1101MinBandwidthKhz = 58.0f;
    static constexpr float kCc1101MaxBandwidthKhz = 812.0f;
    static constexpr float kCc1101MinPowerDbm = -30.0f;
    static constexpr float kCc1101MaxPowerDbm = 10.0f;

    // CC1101 Safe Defaults
    static constexpr float kCc1101DefaultStartMhz = 905.0f;
    static constexpr float kCc1101DefaultStopMhz = 928.0f;
    static constexpr float kCc1101DefaultBandwidthKhz = 500.0f;
    static constexpr float kCc1101DefaultPowerDbm = -1.0f;

    bool isCc1101FrequencyAllowed(float mhz);
    bool isCc1101BandwidthAllowed(float khz);
    bool isCc1101PowerAllowed(float dbm);
    bool isCc1101FrequencyRangeAllowed(float startMhz, float stopMhz);

private:
    HAL();
    Adafruit_NeoPixel* _pixels;
    
    // LED State
    bool _ledPower = true; // Default ON
    uint8_t _r = 0, _g = 0, _b = 0;

    // Hardware Flags
    bool _hasRadio = false;
    bool _hasGPS = false;
    bool _hasMeshtastic = false;

    SPIClass* _radioSPI;
    CC1101* _radio;
};

#endif
