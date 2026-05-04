
#include <Adafruit_NeoPixel.h>

const int NUM_LEDS = 1;

// Adafruit boards with a built-in NeoPixel define PIN_NEOPIXEL in their variant.
// Boards without one (e.g. bare Pico) fall back to pin 0 — leds calls become no-ops.
#ifdef PIN_NEOPIXEL
const int NEOPIXEL_PIN = PIN_NEOPIXEL;
#else
const int NEOPIXEL_PIN = 0;
#endif

Adafruit_NeoPixel leds(NUM_LEDS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

void board_leds_begin() {
    leds.begin();
    leds.setBrightness(32);
    leds.fill(0xffffff);
    leds.show();
}

void board_leds_set(uint32_t color) {
    leds.fill(color);
    leds.show();
}
