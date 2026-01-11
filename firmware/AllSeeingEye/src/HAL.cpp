#include "HAL.h"
#include "Logger.h"

// Hardware Pin Definitions (From README)
#define RGB_LED_PIN 48
#define NUM_PIXELS 1

HAL& HAL::instance() {
    static HAL _instance;
    return _instance;
}

HAL::HAL() : _pixels(nullptr) {}

void HAL::init() {
    Logger::instance().info("HAL", "Initializing Hardware...");

    // LED Init
    _pixels = new Adafruit_NeoPixel(NUM_PIXELS, RGB_LED_PIN, NEO_GRB + NEO_KHZ800);
    _pixels->begin();
    _pixels->setBrightness(50);
    setLed(0, 0, 0); // Off

    // Radio Init (Stub for Phase 4)
    // Serial.println("[HAL] Radio Init skipped (Phase 4)");
}

void HAL::setLed(uint8_t r, uint8_t g, uint8_t b) {
    if (_pixels) {
        _pixels->setPixelColor(0, _pixels->Color(r, g, b));
        _pixels->show();
    }
}

bool HAL::checkRadio() {
    Logger::instance().info("HAL", "POST: Checking Radio...");
    // TODO: Implement actual SPI check in Phase 4
    Logger::instance().info("HAL", "POST: Radio check skipped (Stub).");
    return true; 
}
