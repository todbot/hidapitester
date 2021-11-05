
#include <Adafruit_NeoPixel.h>

const int NUM_LEDS = 1;

#if ADAFRUIT_QTPY_M0

const int NEOPIXEL_PIN = 11;
Adafruit_NeoPixel leds(NUM_LEDS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

#else

//#warning "unknown board, setting NEOPIXEL_PIN to 0"
const int NEOPIXEL_PIN = 0;
Adafruit_NeoPixel leds(NUM_LEDS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

#endif

void board_leds_begin() {
    leds.begin(); // Initialize pins for output
    leds.setBrightness(32);
    leds.fill(0xffffff);
    leds.show();
}

void board_leds_set(uint32_t color) {
    leds.fill(color);
    leds.show();
}
