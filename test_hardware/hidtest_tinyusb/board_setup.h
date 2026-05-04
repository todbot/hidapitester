#ifndef BOARD_SETUP_H
#define BOARD_SETUP_H

#include <Adafruit_NeoPixel.h>

const int NUM_LEDS = 1;

// Adafruit boards with a built-in NeoPixel define PIN_NEOPIXEL in their variant.
// Boards without one (e.g. bare Pico) fall back to pin 0 — leds calls become no-ops.
#ifdef PIN_NEOPIXEL
const int NEOPIXEL_PIN = PIN_NEOPIXEL;
#else
const int NEOPIXEL_PIN = 0;
#endif

const int LED_PIN = LED_BUILTIN;  // the number of the LED pin

Adafruit_NeoPixel leds(NUM_LEDS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

void board_leds_begin() {
    leds.begin();
    leds.setBrightness(32);
    leds.fill(0xffffff);
    leds.show();
    pinMode(LED_PIN, OUTPUT);  // simple on/off LED
}

void board_leds_set(uint32_t color) {
    leds.fill(color);
    leds.show();
    digitalWrite(LED_PIN, color!=0 ? HIGH : LOW);
}

// blocking
void board_leds_blink(uint8_t times, uint16_t blink_time) {
    for(int i=0; i<times; i++) { 
        board_leds_set(0xffffff);
        delay(blink_time);
        board_leds_set(0x000000);
        delay(blink_time);
    }
}

#endif