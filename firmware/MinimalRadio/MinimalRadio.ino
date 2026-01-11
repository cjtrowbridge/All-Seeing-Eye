#include <RadioLib.h>
#include <SPI.h>

SPIClass radioSPI(FSPI);
CC1101 radio = new Module(10, 4, RADIOLIB_NC, 5, radioSPI);

void setup() {
  Serial.begin(115200);
  radioSPI.begin(12, 13, 11, 10);
  int state = radio.begin();
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("Success!");
  } else {
    Serial.print("Failed: ");
    Serial.println(state);
  }
}

void loop() {}
