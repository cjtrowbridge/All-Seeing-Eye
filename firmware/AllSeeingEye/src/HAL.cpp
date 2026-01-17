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
    _ledPower = true; // Default On
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
        _hasRadio = true;
    } else {
        Logger::instance().error("HAL", "Radio Init FAILED code: %d", state);
        _hasRadio = false;
    }
}

void HAL::setLed(uint8_t r, uint8_t g, uint8_t b) {
    // 1. Update State
    _r = r; _g = g; _b = b;

    // 2. Apply to HW only if Powered
    if (_pixels && _ledPower) {
        _pixels->setPixelColor(0, _pixels->Color(r, g, b));
        _pixels->show();
    }
}

void HAL::setLedPower(bool on) {
    _ledPower = on;
    if (on) {
        // Restore current color
        setLed(_r, _g, _b);
    } else {
        // Force Off (but don't lose color state)
        if (_pixels) {
            _pixels->setPixelColor(0, 0); // Black
            _pixels->show();
        }
    }
}

bool HAL::getLedPower() {
    return _ledPower;
}

uint32_t HAL::getLedColor() {
    // Return standard RGB integer
    return ((uint32_t)_r << 16) | ((uint32_t)_g << 8) | _b;
}

CC1101* HAL::getRadio() {
    return _radio;
}

bool HAL::checkRadio() {
    // Return the cached POST result
    return _hasRadio; 
}

bool HAL::hasRadio() { return _hasRadio; }
bool HAL::hasGPS() { return _hasGPS; }
bool HAL::hasMeshtastic() { return _hasMeshtastic; }

bool HAL::isCc1101FrequencyAllowed(float mhz) {
    if (mhz >= kCc1101Band1MinMhz && mhz <= kCc1101Band1MaxMhz) return true;
    if (mhz >= kCc1101Band2MinMhz && mhz <= kCc1101Band2MaxMhz) return true;
    if (mhz >= kCc1101Band3MinMhz && mhz <= kCc1101Band3MaxMhz) return true;
    return false;
}

bool HAL::isCc1101FrequencyRangeAllowed(float startMhz, float stopMhz) {
    if (startMhz >= kCc1101Band1MinMhz && stopMhz <= kCc1101Band1MaxMhz) return true;
    if (startMhz >= kCc1101Band2MinMhz && stopMhz <= kCc1101Band2MaxMhz) return true;
    if (startMhz >= kCc1101Band3MinMhz && stopMhz <= kCc1101Band3MaxMhz) return true;
    return false;
}

bool HAL::isCc1101BandwidthAllowed(float khz) {
    return khz >= kCc1101MinBandwidthKhz && khz <= kCc1101MaxBandwidthKhz;
}

bool HAL::isCc1101PowerAllowed(float dbm) {
    return dbm >= kCc1101MinPowerDbm && dbm <= kCc1101MaxPowerDbm;
}
