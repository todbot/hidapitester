/**
 * hid_generic_trinketm0  
 * -- general purpose HID device that handles IN/OUT/FEATURE reports 
 *     with or without Report IDs
 * 
 * To use:
 * - Use Trinket M0 or other device supported by Adafruit_TinyUSB
 * - Install "Adafruit SAMD Board" board package with Boards Manager
 * - Install "Adafruit_TinyUSB" library with Library Manager
 * - In Arduino IDE, select Adafruit SAMD-based board (e.g. "Trinket M0")
 * - In Arduino IDE, set Tools->USB Stack->TinyUSB
 * 
 */
 
/*********************************************************************
 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 Copyright (c) 2019 Ha Thach for Adafruit Industries
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/

/* This example demonstrate HID Generic raw Input & Output.
 * It will receive data from Host (In endpoint) and echo back (Out endpoint).
 * HID Report descriptor use vendor for usage page (using template TUD_HID_REPORT_DESC_GENERIC_INOUT)
 *
 * There are 2 ways to test the sketch
 * 1. Using nodejs
 *    - Install nodejs and nmp to your PC
 *    - Install execellent node-hid (https://github.com/node-hid/node-hid) by
 *      $ npm install node-hid
 *    - Run provided hid test script 
 *      $ node hid_test.js
 *    
 * 2. Using python hidRun 
 *    - Python and `hid` package is required, for installation please follow https://pypi.org/project/hid/
 *    - Run provided hid test script to send and receive data to this device.
 *      $ python3 hid_test.py
 */

#include "Adafruit_TinyUSB.h"

// pick one
#define MODE  MODE_TEENSY
//#define MODE  MODE_BLINK1

#include "descriptors.h"

// HID report descriptor using descriptors.h template
uint8_t const desc_hid_report[] = { HID_DESC };

Adafruit_USBD_HID usb_hid;

int size0 = 0;
int size1 = 0;

// the setup function runs once when you press reset or power the board
void setup()
{
    USBDevice.setID( VID, PID );

    usb_hid.enableOutEndpoint(true);
    usb_hid.setPollInterval(2);
    usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
    usb_hid.setReportCallback(get_report_callback, set_report_callback);

    usb_hid.begin();

    Serial.begin(115200);

    // wait until device mounted
    while( !USBDevice.mounted() ) delay(1);

//    delay(1000);
//    Serial.println("hid_generic_trinketm0 hidapitester");
}

void loop()
{
    Serial.printf("MODE:%s... size0:%d, size1:%d\n",MODE_STR,size0,size1); 
    if( Serial.available() ) { 
        int c = Serial.read();
        Serial.printf("read:%c\n",(char)c);
    }
    delay(3000);
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t get_report_callback (uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
    Serial.print("Got GET_REPORT request:");
    Serial.print(" report_id:"); Serial.print(report_id);
    Serial.print(" report_type:"); Serial.print(report_type);
    
    Serial.print(" reqlen:"); Serial.print(reqlen);
    Serial.println();
    
    buffer[0] = report_id;  //reportId
    buffer[1] = 'a';
    buffer[2] = 'b';
    buffer[3] = 'c';
    buffer[4] = '1';
    buffer[5] = '2';
    buffer[6] = '3';
    buffer[7] = '4';
    return reqlen;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void set_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
    Serial.print("Got SET_REPORT request:");
    Serial.print(" report_id:"); Serial.print(report_id);
    Serial.print(" report_type:"); Serial.print(report_type);
    Serial.print(" bufsize:"); Serial.print(bufsize);
    Serial.println();
    for( int i=0; i< bufsize; i++) {
        Serial.print(buffer[i],HEX); Serial.print(",");
    }
    Serial.println();

    // echo back anything we received from host
    //usb_hid.sendReport(0, buffer, bufsize);
}
