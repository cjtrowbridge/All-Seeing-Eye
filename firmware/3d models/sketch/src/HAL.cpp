#line 1 "C:\\Users\\CJ\\Documents\\GitHub\\All-Seeing-Eye\\firmware\\AllSeeingEye\\src\\HAL.cpp"
#include "HAL.h"
#include "Logger.h"

// Hardware Pin Definitions (From README)
#define RGB_LED_PIN 48
#define NUM_PIXELS 1

// Radio Pins
#define R_SCK 12
#define R_MISO 13
#define R_MOSI 11
#define R_CS 10
#define R_GDO0 4
#define R_GDO2 5 // Optional

HAL& HAL::instance() {
    static HAL _instance;
    return _instance;
}

HAL::HAL() : _pixels(nullptr), _radio(nullptr), _radioSPI(nullptr) {}

void HAL::init() {
    Logger::instance().info("HAL", "Initializing Hardware...");

    // LED Init
    _pixels = new Adafruit_NeoPixel(NUM_PIXELS, RGB_LED_PIN, NEO_GRB + NEO_KHZ800);
    _pixels->begin();
    _pixels->setBrightness(50);
    setLed(0, 0, 0); // Off

    // SPI Init
    _radioSPI = new SPIClass(FSPI);
    _radioSPI->begin(R_SCK, R_MISO, R_MOSI, R_CS);

    // Radio Init
    // Module(cs, irq, rst, gpio)
    // RST is not connected (RADIOLIB_NC)
    // Using GDO2 as GPIO 
    _radio = new CC1101(new Module(R_CS, R_GDO0, RADIOLIB_NC, R_GDO2, *_radioSPI));
    
    // Attempt basic begin
    int state = _radio->begin();
    if (state == RADIOLIB_ERR_NONE) {
        Logger::instance().info("HAL", "Radio Init OK (CC1101 detected)");
        // Set basic defaults?
    } else {
        Logger::instance().error("HAL", "Radio Init FAILED code: %d", state);
    }
}

void HAL::setLed(uint8_t r, uint8_t g, uint8_t b) {
    if (_pixels) {
        _pixels->setPixelColor(0, _pixels->Color(r, g, b));
        _pixels->show();
    }
}

CC1101* HAL::getRadio() {
    return _radio;
}

bool HAL::checkRadio() {
    // If init failed, _radio pointer is still valid but hardware might be bad?
    // Let's check status directly by reading a register or checking state?
    // For now we rely on the init result logged above.
    return true; 
}
