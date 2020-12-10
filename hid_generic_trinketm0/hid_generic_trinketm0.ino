/**
 * hid_generic_trinketm0  
 * -- general purpose HID device that handles IN/OUT/FEATURE reports 
 *     with or without Report IDs
 * 
 * To use:
 * - Use Trinket M0 or other device supported by the "Adafruit_TinyUSB" library
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

#include <Adafruit_TinyUSB.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_DotStar.h>

// pick one
//#define MODE  MODE_INOUT_NO_REPORTID
#define MODE  MODE_INOUT_WITH_REPORTIDS
//#define MODE  MODE_FEATURE_NO_REPORTID
//#define MODE  MODE_FEATURE_WITH_REPORTIDS
//#define MODE  MODE_BLINK1
//#define MODE  MODE_TEENSY

#include "descriptors.h"

// HID report descriptor using descriptors.h template
uint8_t const desc_hid_report[] = { HID_DESC };

Adafruit_USBD_HID usb_hid;

uint32_t statusMillisNext;

const int DOTSTAR_DAT = 7; 
const int DOTSTAR_CLK = 8; 

Adafruit_DotStar dotstrip = Adafruit_DotStar( 1, DOTSTAR_DAT, DOTSTAR_CLK, DOTSTAR_BGR);


// the setup function runs once when you press reset or power the board
void setup()
{
    USBDevice.setID( VID, PID ); // VID,PID set in 'descriptors.h'
//    USBDevice.setSerialNumberDescriptor("1234");

    usb_hid.enableOutEndpoint(true);
    usb_hid.setPollInterval(2);
    usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
    usb_hid.setReportCallback(get_report_callback, set_report_callback);

    usb_hid.begin();

    dotstrip.begin(); // Initialize pins for output
    dotstrip.setPixelColor(0,0x330033);
    dotstrip.show();

    Serial.begin(115200);
    Serial.setTimeout(500);  // for serial.readString()
    
    // wait until device mounted
    while( !USBDevice.mounted() ) delay(1);

//    delay(1000);
//    Serial.println("hid_generic_trinketm0 hidapitester");
}

uint8_t buf[64];
char tmpstr[100];

void loop()
{
    if( statusMillisNext - millis() > 3000 ) { 
        Serial.printf("MODE:%s... \n",MODE_STR); 
        statusMillisNext = millis() + 3000;
        dotstrip.setPixelColor(0,0x330033);
        dotstrip.show();
    }
    
    if( Serial.available() ) { 
        String str = Serial.readString();
        str.getBytes( (unsigned char*)tmpstr, 100);
        Serial.printf("serial read str:%s\n",tmpstr);
        int c = hexread(buf, tmpstr, 64 );
        if( c > 0 ) {
            Serial.printf("serial read buf: len=%d\n",c);
            for( int i=0; i<c; i++ ) { 
                Serial.printf("%02.2x,",buf[i]);
            }
            Serial.println();
        }
       usb_hid.sendReport(1, buf, sizeof(buf));
       memset(buf,0,sizeof(buf));
    }
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t get_report_callback (uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
    dotstrip.setPixelColor(0,0x000033);
    dotstrip.show();
    
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
    dotstrip.setPixelColor(0,0x000033);
    dotstrip.show();
    
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
//    usb_hid.sendReport(0, buffer, bufsize);
//    usb_hid.sendReport(report_id, buffer, bufsize);
}

// parse a comma-delimited string containing numbers (dec,hex) into a byte arr buffer
int hexread(uint8_t *buffer, char *string, int buflen)
{
    char* s;
    int pos = 0;
    if( string==NULL ) return -1;
    memset(buffer,0,buflen); 
    while((s = strtok(string, ", ")) != NULL && pos < buflen){
        string = NULL;
        buffer[pos] = (char)strtol(s, NULL, 0);
        pos++;
    }
    return pos;
}
