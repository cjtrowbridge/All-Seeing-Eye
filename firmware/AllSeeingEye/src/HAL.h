#ifndef HAL_H
#define HAL_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
// #include <RadioLib.h> // Will implement in Phase 4

class HAL {
public:
    static HAL& instance();
    
    void init();
    void setLed(uint8_t r, uint8_t g, uint8_t b);
    bool checkRadio(); // Power-On Self Test

private:
    HAL();
    Adafruit_NeoPixel* _pixels;
};

#endif
