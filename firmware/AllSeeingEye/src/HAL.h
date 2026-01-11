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
    void setLed(uint8_t r, uint8_t g, uint8_t b);
    bool checkRadio(); // Power-On Self Test
    
    // Radio Access
    CC1101* getRadio();

private:
    HAL();
    Adafruit_NeoPixel* _pixels;
    SPIClass* _radioSPI;
    CC1101* _radio;
};

#endif
