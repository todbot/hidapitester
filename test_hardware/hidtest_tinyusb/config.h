#ifndef HIDTEST_TINYUSB_CONFIG_H
#define HIDTEST_TINYUSB_CONFIG_H

#include <Arduino.h>
#include "hid_settings.h"

typedef struct {
  boolean valid;
  uint8_t hid_mode;
} StartupConfig;

extern StartupConfig config;

#if defined(ARDUINO_ARCH_ESP32)

#include <Preferences.h>
Preferences prefs;

int load_config() {
    prefs.begin("hidtest", true);
    config.valid   = prefs.getBool("valid", false);
    config.hid_mode = prefs.getInt("mode", DEFAULT_HID_MODE);
    prefs.end();
    if (!config.valid || config.hid_mode >= HID_MODE_COUNT) { config.hid_mode = DEFAULT_HID_MODE; config.valid = true; return -1; }
    return 0;
}
void save_config() {
    prefs.begin("hidtest", false);
    prefs.putBool("valid", true);
    prefs.putInt("mode", config.hid_mode);
    prefs.end();
}
#else  // RP2040, SAMD and others

#include <EEPROM.h>

int load_config() {
    EEPROM.begin(sizeof(StartupConfig));
    EEPROM.get(0, config);
    if (!config.valid || config.hid_mode >= HID_MODE_COUNT) { config.hid_mode = DEFAULT_HID_MODE; config.valid = true; return -1; }
    return 0;
}
void save_config() {
    EEPROM.put(0, config);
    EEPROM.commit();
}

#endif  // storage backend

#endif  // HIDTEST_TINYUSB_CONFIG_H