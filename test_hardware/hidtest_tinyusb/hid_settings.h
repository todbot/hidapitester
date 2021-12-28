#ifndef TOD_HID_SETTINGS_H
#define TOD_HID_SETTINGS_H

#include "descriptors.h"

typedef struct
{
    // USB information
    uint16_t vid;
    uint16_t pid;
    const char* manufacturer_str;
    const char* product_str;

    // USB HID Report Descriptor
    const uint8_t* desc_hid_report;
    const uint8_t desc_size;

    // information for us humans using this sketch
    const char* info; 
} HIDSetting;

const uint8_t hid_report_inout_noid_32[] = { HID_DESC_INOUT_NO_REPORTID( 32 ) };
const uint8_t hid_report_inout_id_32[] = { HID_DESC_INOUT_WITH_REPORTID( 32 ) };
const uint8_t hid_report_teensy[] = { HID_DESC_TEENSY_RAWHID() };
const uint8_t hid_report_blink1[] = { HID_DESC_BLINK1MK2() };

const HIDSetting setting_inout_noid_32 =
{
 .vid = 0x27b8,
 .pid = 0xee32,
 .manufacturer_str = "hidapitester",
 .product_str = "INOUT 32bytes",
 .desc_hid_report = hid_report_inout_noid_32,
 .desc_size = sizeof(hid_report_inout_noid_32),
 .info = "IN/OUT reports 32 bytes, no reportIds"
};

const HIDSetting setting_inout_id_32 =
{
 .vid = 0x27b8,
 .pid = 0xee33,
 .manufacturer_str = "hidapitester",
 .product_str = "INOUT 32bytes rId1",
 .desc_hid_report = hid_report_inout_id_32,
 .desc_size = sizeof(hid_report_inout_id_32),
 .info = "IN/OUT reports 32 bytes, with reportId 1"
};

const HIDSetting setting_teensy =
{
  .vid = 0x27b8,
  .pid = 0xeeee,
  .manufacturer_str = "hidapitester",
  .product_str = "FakeTeensy",
  .desc_hid_report = hid_report_teensy,
  .desc_size = sizeof(hid_report_teensy),
  .info = "Teensy RAWHID like, 64-byte IN/OUT reports, no reportId"
};

const HIDSetting setting_blink1 =
{
 .vid = 0x27b8,
 .pid = 0x4444,
 .manufacturer_str = "hidapitester",
 .product_str = "blink(1) in name only",
 .desc_hid_report = hid_report_blink1,
 .desc_size = sizeof(hid_report_blink1),
 .info = "blink(1) like, FEATURE reports 1 (8-bytes) and 2 (60-bytes)"
};


const HIDSetting settings[] =
{
 setting_inout_noid_32,
 setting_inout_id_32,
 setting_teensy,
 setting_blink1,
};


#endif
