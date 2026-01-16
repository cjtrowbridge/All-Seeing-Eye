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
