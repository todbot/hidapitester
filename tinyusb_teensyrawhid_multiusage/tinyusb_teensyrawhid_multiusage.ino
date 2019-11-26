/**
 * tinyusb_teensyrawhid_multiusage- TinyUSB clone of Teensy RawHID example, with two usages
 * 
 *   When programmed on a TinyUSB-compatible device (Trinket M0, etc),
 *   this sketch provides a HID device that attempts to be a work-alike clone
 *   of the Teensy RawHID sketch (https://www.pjrc.com/teensy/rawhid.html)
 *   It has the properties:
 *   - VID/PID same as Teensy
 *   - usage/usagePage same as Teensy RawHID
 *   - HID descriptor advertises one 64-byte Input report & one 64-byte Output report
 *   - No reportIds in use
 *   - No Feature reports in use 
 *   When running, the sketch will do:
 *   - print out any received data received from host (OUT / SET_REPORT)
 *   - 
 *   
 *   Because of how TinyUSB differs from Teensy USB, 
 *   there is no "Emulated Serial" HID device, but there is 
 *   an actual CDC UART device (and during bootloader, a MSC device)
 * 
 * To compile:
 * - Use a TinyUSB-compatible board (Trinket M0, etc)
 * - Install "Adafruit SAMD Board" board package with Boards Manager
 * - Install "Adafruit_TinyUSB" library with Library Manager
 * - In Arduino IDE, select Adafruit SAMD-based board (e.g. "Trinket M0")
 * - In Arduino IDE, set Tools->USB Stack->TinyUSB
 * 
 * Part of hidapitester - https://github.com/todbot/hidapitester
 * 2019 @todbot Tod E. Kurt
 * 
 */

#include <Adafruit_TinyUSB.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_DotStar.h>

// "descriptors.h" contains the report-specific details
#include "descriptors.h"

// HID report descriptor using descriptors.h template
uint8_t const desc_hid_report[] = { HID_DESC };

// pins used by this sketch
const int DOTSTAR_DAT = 7; 
const int DOTSTAR_CLK = 8; 

Adafruit_DotStar dotstrip = Adafruit_DotStar( 1, DOTSTAR_DAT, DOTSTAR_CLK, DOTSTAR_BGR);

Adafruit_USBD_HID usb_hid;

uint32_t statusMillisNext;

// the setup function runs once when you press reset or power the board
void setup()
{
    USBDevice.setID( MYVID, MYPID ); // MYVID,MYPID set in 'descriptors.h'
    // these two don't quite work yet
    //USBDevice.setManufacturerDescriptor("NotPJRC");
    //USBDevice.setProductDescriptor("Teensy2Clone");
    
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

}

uint8_t buf[RAWHID_TXRX_SIZE];
char tmpstr[100];

void sendReport()
{
  uint32_t now = millis();
  buf[0] = 0xab;
  buf[1] = 0xcd;
  buf[2] = 0xef;
  buf[3] = 0x00;
  buf[4] = (now >>  0) & 0xff;
  buf[5] = (now >>  8) & 0xff;
  buf[6] = (now >> 16) & 0xff;
  
  Serial.println("Sending buf:");
  printbuf(buf,sizeof(buf));
  usb_hid.sendReport(0, buf, sizeof(buf)); // reportId=0 == no reportId
}

void loop()
{
    if( statusMillisNext - millis() > 3000 ) { 
        Serial.printf("\nMODE:%s Type comma-sep list of bytes to send...\n",MODE_STR); 
        statusMillisNext = millis() + 3000;
        dotstrip.setPixelColor(0,0x330033);
        dotstrip.show();

        sendReport();
    }
    
    if( Serial.available() ) { 
        String str = Serial.readString();
        str.getBytes( (unsigned char*)tmpstr, 100);
        Serial.printf("serial read str:%s\n",tmpstr);
        int c = hexread(buf, tmpstr, sizeof(buf) );
        if( c > 0 ) {
            Serial.printf("Sending serial read buf: len=%d\n",c);
            printbuf(buf, sizeof(buf));
            Serial.println();
        }
       usb_hid.sendReport(0, buf, sizeof(buf)); // reportId=0 == no reportId
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
    
    Serial.printf("Got GET_REPORT request:");
    Serial.printf(" report_id:%d",report_id);
    Serial.printf(", report_type:%d",report_type);
    Serial.printf(", reqlen:%d", reqlen);
    Serial.println();
    
    buffer[0] = report_id;  //reportId
    buffer[1] = 'a';
    buffer[2] = 'b';
    buffer[3] = 'c';
    buffer[4] = '1';
    buffer[5] = '2';
    buffer[6] = '3';
    buffer[7] = '4';

    Serial.printf("Sending to host:\n");
    printbuf(buffer,reqlen);
    Serial.println();
    
    return reqlen;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void set_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
    dotstrip.setPixelColor(0,0x000033);
    dotstrip.show();
    
    Serial.printf("Got SET_REPORT request:");
    Serial.printf(" report_id:%d",report_id);
    Serial.printf(", report_type:%d",report_type);
    Serial.printf(", bufsize:%d",bufsize);
    Serial.println();
    printbuf( buffer, bufsize );
//    for( int i=0; i< bufsize; i++) {
//        Serial.printf("%2.2x,", buffer[i]);
//    }
    Serial.println();
}

// print out a byte buffer to serial port
void printbuf(const uint8_t* buf, int buflen)
{
  int width = 32;
  for( int i=0; i<buflen; i++ ) { 
    Serial.printf(" %02X",buf[i]);
    if (i % width == width-1 && i < buflen-1) {
      Serial.println();
    }
  }
  Serial.println();
}

// parse a comma-delimited string containing numbers (dec,hex) into a byte arr buffer
// e.g. "6,5,15,64,2,0"  -> {0x06, 0x05, 0x0f, 0x40, 0x02, 0x00} 
// e.g. "0x33,0xff,1,16" -> {0x33, 0xff, 0x01, 0x10}
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
