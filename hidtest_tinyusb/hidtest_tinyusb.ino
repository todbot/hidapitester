/**
 * hidtest_tinyusb.ino
 * -- general purpose HID device that handles IN/OUT/FEATURE reports, 
 *     with or without Report IDs
 *
 * To compile:
 * - Use QTPy M0, Trinket M0, or other device supported by "Adafruit_TinyUSB"
 * - Install "Adafruit SAMD Board" board package with Boards Manager
 * - Install "Adafruit_TinyUSB" library with Library Manager
 * - In Arduino IDE, select Adafruit SAMD-based board (e.g. "Trinket M0")
 * - In Arduino IDE, set Tools->USB Stack->TinyUSB
 *
 * To use:
 * - Set the MODE define to one of the available HID Report Descriptor types
 * - Upload sketch to board
 * - Open the Serial Monitor to see reports received
 * - To send reports, use Serial monitor to send a string that looks like:
 *    - Send 64-byte INPUT report w/ no reportId w/ first 4 bytes 11,22,33,44:
 *       I 65 0x00 0x11 0x22 0x33 0x44 
 *    - Send 8-byte FEATURE report on reportId 1:
 *       F 9  0x01 0x11 0x22 0x33 0x44 0x55 0x66 0x77 0x88
 *    - The first character is report type to send
 *    - The next number is how many bytes to send (one more than report size)
 *    - The remaining numbers (hex or dec) are the bytes to send in the report,
 *       any undefined bytes are set to 0x00.
 *      
 * - Those are two examples of the different "commands" this sketch understands
 *   via its serial input.  The full list of commands are:
 *    - I - send INPUT report to host
 *    - F - send FEATURE report to host
 *    - E - Turn report echo on/off
 *    - H - print help
 *    
 */

// Config: Pick one of these
//#define MODE  MODE_INOUT_NO_REPORTID
//#define MODE  MODE_INOUT_WITH_REPORTIDS
//#define MODE  MODE_FEATURE_NO_REPORTID
#define MODE  MODE_FEATURE_WITH_REPORTID
//#define MODE  MODE_BLINK1
//#define MODE  MODE_TEENSY

#include <Adafruit_TinyUSB.h>
#include <Adafruit_NeoPixel.h>

#include "descriptors.h"

// HID report descriptor using descriptors.h template
uint8_t const desc_hid_report[] = { HID_DESC };

Adafruit_USBD_HID usb_hid;

uint32_t statusMillisNext;
bool echoReports = false;

const int NUM_LEDS = 1;
#if ADAFRUIT_QTPY_M0
const int NEOPIXEL_PIN = 11;
Adafruit_NeoPixel leds(NUM_LEDS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
#else
#warning "unknown board, setting NEOPIXEL_PIN to 0"
const int NEOPIXEL_PIN = 0;
Adafruit_NeoPixel leds(NUM_LEDS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
#endif

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

    leds.begin(); // Initialize pins for output
    leds.setPixelColor(0,0x330033);
    leds.show();

    Serial.begin(115200);
    Serial.setTimeout(500);  // for serial.readString()
    
    // wait until device mounted
    while( !USBDevice.mounted() ) delay(1);

    delay(1000);
    Serial.println("hidtest_tinyusb hidapitester");
    Serial.println("hid report descriptor:");
    printbuf(desc_hid_report, sizeof(desc_hid_report));
}

void help()
{
    Serial.println("Here is where help would be\n");
}

// clear output the input buffer
void drainSerial()
{
    while( Serial.read() != -1 )  { } // drain any extra
}

void loop()
{
    if( statusMillisNext - millis() > 3000 ) {
        Serial.printf("hidtest_tinyusb vidpid=%4X:%4X mode=%s... ('?' for help)\n",
                      VID,PID, MODE_STR);
        statusMillisNext = millis() + 3000;
        leds.setPixelColor(0,0x330033);
        leds.show();
    }

    // Serial monitor commands
    if( Serial.available() ) {
        char cmd = tolower( Serial.read() );
        if( cmd == '?' || cmd == 'h') {
            help();
            drainSerial();
        }
        else if( cmd == 'e' ) { // echo on/off
            int onoff = Serial.parseInt();
            echoReports = onoff;
            Serial.printf("Setting echoReports to %d\n", echoReports);
            drainSerial();
        }
        else if( cmd == 'f' || cmd == 'i' ) { // send feature or input report
            int len = Serial.parseInt();
            if( len==0 ) {
                Serial.println("Invalid report length specified");
            }
            Serial.printf("cmd: %c len:%d\n", cmd,len);
            
            // FIXME: danger, fixed-size buffs below
            String str = Serial.readString();
            char cstr[100]; // used to convert String to char
            uint8_t tmpbuf[64]; // buffer of bytes converted from cstr
            memset(tmpbuf, 0, sizeof(tmpbuf)); // ensure all zeros
            str.getBytes( (unsigned char*)cstr, sizeof(cstr));
            Serial.printf("serial read:'%s'\n",cstr);
            int cnt = hexread( cstr, tmpbuf, sizeof(tmpbuf) );
            if( cnt > 0 ) {
                Serial.printf("Serial read buf: len=%d\n",cnt);
                usb_hid.sendReport( tmpbuf[0], tmpbuf+1, len-1);
                memset(tmpbuf,0,sizeof(tmpbuf)); // clear it out after use
            }
        } else {
            Serial.printf("unrecognized cmd: %c\n",cmd);
        }
    }
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
// sent FEATURE report (report_type == 3)
uint16_t get_report_callback (uint8_t report_id, hid_report_type_t report_type, 
                              uint8_t* buffer, uint16_t reqlen) {
    leds.setPixelColor(0,0x000033);
    leds.show();

    Serial.print("GET_REPORT request:");
    Serial.printf(" report_id: %d",report_id);
    Serial.printf(" report_type:%d (%s)",report_type,
                  (report_type==3) ? "FEATURE" : "OTHER");
    
    Serial.print(" reqlen:"); Serial.print(reqlen);
    Serial.println();
    
    //buffer[0] = report_id; //reportId no reportid implied in this
    //case
    buffer[0] = 'a';
    buffer[1] = 'b';
    buffer[2] = 'c';
    buffer[3] = '1';
    buffer[4] = '2';
    buffer[5] = '3';
    buffer[6] = '4';
    
    Serial.printf("Sending buffer for reportId:%d:\n", report_id);
    printbuf(buffer, reqlen);
    
    return reqlen;
}

// Invoked when received SET_REPORT control request or
// received OUTPUT report (report_type == 0 )
// received FEATURE report (report_type == 3)
void set_report_callback(uint8_t report_id, hid_report_type_t report_type, 
                          uint8_t const* buffer, uint16_t bufsize) {
    leds.setPixelColor(0,0x000033);
    leds.show();
    
    Serial.print("SET_REPORT :");
    Serial.print(" report_id:"); Serial.print(report_id);
    Serial.print(" report_type:"); Serial.print(report_type);
    Serial.print(" bufsize:"); Serial.print(bufsize);
    Serial.println();
    printbuf(buffer, bufsize);
    Serial.println();
    
    // echo back anything we received from host
    if( echoReports ) { 
        usb_hid.sendReport(0, buffer, bufsize);
    }
}

// print out a byte buffer to serial port
void printbuf(const uint8_t* buf, int buflen)
{
  int screen_width = 32;
  for( int i=0; i<buflen; i++ ) { 
    Serial.printf(" %02X",buf[i]);
    if (i % screen_width == screen_width-1 && i < buflen-1) {
      Serial.println();
    }
  }
  Serial.println();
}

// parse a comma-delimited 'string' containing numbers (dec,hex)
// into a byte array 'buffer'
// returns number of numbers read
int hexread(char *string, uint8_t *buffer, int buflen)
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
