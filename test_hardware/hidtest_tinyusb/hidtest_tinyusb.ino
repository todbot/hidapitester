/**
 * hidtest_tinyusb.ino
 * -- Arduino sketch for TinyUSB-compatible SAMD21/51 devices, to create various 
 *    general purpose HID device that handles IN/OUT/FEATURE reports, 
 *    with or without Report IDs
 *
 * To compile:
 * - Use QTPy M0, Trinket M0, or other device supported by "Adafruit_TinyUSB"
 * - Install "Adafruit SAMD Board" board package with Boards Manager
 * - Install following libraries with Library Manager
 *    - Adafruit_TinyUSB" (version 1.6.0 or better)
 *    - Adafruit_SleepyDog
 *    - FlashStorage
 *    - AdafruitNeopixel
 * - In Arduino IDE, select Adafruit SAMD-based board (e.g. "Trinket M0")
 * - In Arduino IDE, set Tools -> USB Stack -> TinyUSB
 *
 * FIXME: The below needs to be updated
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
 *    - I   - send INPUT report to host
 *    - F   - send FEATURE report to host
 *    - E   - Turn report echo on/off
 *    - H/? - print help
 * 
 * Examples:
 * - Set MODE == MODE_INOUT_NO_REPORTID  (64-byte, no reportIds)
 *    - hidapitester:
 *     hidapitester --vidpid=239A:801E -l 65 -t 1500 --send-output 0,1,2,3,4,5,6
 * 
 * - Set MODE == MODE_FEATURE_WITH_REPORTID
 *    - use "hidapitester" like:
 *      - hidapitester --vidpid=239A:801E -l 33 -t 1500 \
 *             --open --send-feature 1,1,2,3,4,5,6 --read-feature 1
 */

#include <Adafruit_TinyUSB.h>
#include <Adafruit_SleepyDog.h>
#include <FlashStorage.h>  // only samd21 & samd51 currently 

#include "hid_settings.h"

#include "board_config.h"

Adafruit_USBD_HID usb_hid;
                          
uint32_t statusMillisNext;
bool echoReports = false;

int hid_mode = 0;

typedef struct {
  boolean valid;
  int hid_mode;
} StartupConfig;

FlashStorage(config_flash_store, StartupConfig);
StartupConfig config;

// load the saved config from flash
int load_config() {
  config = config_flash_store.read();
  if ( !config.valid ) {   // for first run, "valid" will be "false"...
      config.hid_mode = 0;
      config.valid = true;
      return -1;  // had to fix the config
  }
  return 0; // laod was okay
}

// the setup function runs once when you press reset or power the board
void setup()
{
    Serial.begin(115200);
    Serial.setTimeout(500);  // for serial.readString()
    
    load_config();
    
    HIDSetting setting = settings[ config.hid_mode ];
    
    TinyUSBDevice.setID( setting.vid, setting.pid );
    TinyUSBDevice.setManufacturerDescriptor( setting.manufacturer_str );
    TinyUSBDevice.setProductDescriptor( setting.product_str );

    usb_hid.enableOutEndpoint(true);
    usb_hid.setPollInterval(2);
    usb_hid.setReportDescriptor(setting.desc_hid_report, setting.desc_size);
    usb_hid.setReportCallback(get_report_callback, set_report_callback);
    usb_hid.begin();

    // wait until device mounted
    while( !TinyUSBDevice.mounted() ) delay(1);

    board_leds_begin();

    delay(2000);

    Serial.println("hidtest_tinyusb");
    Serial.println("---------------");
    print_config();
    print_help();    
}

void print_config()
{
    HIDSetting setting = settings[config.hid_mode];
    
    Serial.printf("Current mode: %d ('%s') ", config.hid_mode, setting.info);
    Serial.printf(" vidpid=%04X:%04X\n", setting.vid, setting.pid);
    Serial.printf("Current HID report descriptor: len=%d\n", setting.desc_size);
    print_buff(setting.desc_hid_report, setting.desc_size, "  ");
    Serial.println();
    Serial.println("Available modes:");
    for( uint8_t i=0; i < 4; i++ ) { // FIXME: why can't I use sizeof(settings) here
        HIDSetting s = settings[i];
        Serial.printf("  Mode:%d ", i);
        Serial.printf("  ('%s')\n", s.info);
        Serial.printf("    vid/pid/mfg/prod:");
        Serial.printf(" %04X/%04X:", s.vid,s.pid);
        Serial.printf(" %s - %s\n", s.manufacturer_str, s.product_str);
    }
    Serial.println();
}

void print_help()
{
    HIDSetting setting = settings[config.hid_mode];
    Serial.print("Available commands:\n"
                 "  - i   - send INPUT report to host\n"
                 "  - f   - send FEATURE report to host\n"
                 "  - e   - Turn report echo on/off\n"
                 "  - m   - Select device mode (causes device reset)\n"
                 "  - c   - Show current device config and available modes\n"
                 "  - ?   - Print this help\n");
    Serial.printf("hidtest_tinyusb: hid_mode:%d ",config.hid_mode);
    Serial.printf(" vidpid=%04X:%04X\n", setting.vid, setting.pid);
}

void loop()
{
    HIDSetting setting = settings[config.hid_mode];
    if( statusMillisNext - millis() > 10000 ) {
        statusMillisNext = millis() + 10000;
        board_leds_set(0xff00ff);
        Serial.println("('?' for help)>");
    }
    
    // Serial monitor commands
    if( Serial.available() ) {
        char cmd = tolower( Serial.read() );
        if( cmd == '?' || cmd == 'h') {           // help
            print_help();
            drain_serial();
        }
        else if( cmd == 'c' ) {
            print_config();
            drain_serial();
        }
        else if( cmd == 'm' ) {                   // change device mode
            uint16_t m = Serial.parseInt();
            m = constrain(m, 0, sizeof(settings)-1);
            Serial.printf("Resetting board to mode=%d...\n",m);
            config.hid_mode = m;
            config_flash_store.write(config);
            drain_serial();
            delay(1000);
            TinyUSBDevice.detach();
            Watchdog.enable(1000); // resets after 2000msec
        }
        else if( cmd == 'e' ) {                  // echo on/off
            int onoff = Serial.parseInt();
            echoReports = onoff;
            Serial.printf("Setting echoReports to %d\n", echoReports);
            drain_serial();
        }
        else if( cmd == 'f' || cmd == 'i' ) {    // send feature or input report
            int len = Serial.parseInt();
            if( len==0 ) {
                Serial.println("Invalid report length specified");
            }
            //Serial.printf("cmd: %c len:%d\n", cmd,len);
            
            // FIXME: danger, fixed-size buffs below
            String str = Serial.readString();
            char cstr[100]; // used to convert String to char
            uint8_t tmpbuf[64]; // buffer of bytes converted from cstr
            memset(tmpbuf, 0, sizeof(tmpbuf)); // ensure all zeros
            str.getBytes( (unsigned char*)cstr, sizeof(cstr));
            int cnt = hexread( cstr, tmpbuf, sizeof(tmpbuf) );
            if( cnt > 0 ) {
                Serial.printf("|-- Serial read buf: len=%d\n",cnt);
                print_buff(tmpbuf, cnt, "    ");
                usb_hid.sendReport( tmpbuf[0], tmpbuf+1, len-1);
                memset(tmpbuf,0,sizeof(tmpbuf)); // clear it out after use
            }
        }
        else {
            Serial.printf("Unrecognized cmd: %c\n",cmd);
            drain_serial();
            print_help();
        }
    }
}

// FIXME: 
uint8_t send_buff[64];

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
// sent FEATURE report (report_type == 3)
uint16_t get_report_callback (uint8_t report_id, hid_report_type_t report_type, 
                              uint8_t* buffer, uint16_t reqlen) {
    board_leds_set(0x0000FF);

    Serial.printf("REQUEST %s GET_REPORT:", (report_type==3) ? "FEATURE" : "OTHER");
    Serial.printf(" report_id: %d",report_id);
    Serial.printf(" report_type:%d",report_type);
    Serial.printf(" reqlen:%d\n",reqlen);

    if( echoReports ) {
        Serial.println("|-- Echoing previously received report");
        memcpy(buffer, send_buff, reqlen);
    }
    else {
        buffer[0] = 'a';
        buffer[1] = 'b';
        buffer[2] = 'c';
        buffer[3] = 'd';
        buffer[4] = '1';
        buffer[5] = '2';
        buffer[6] = '3';
        buffer[7] = '4';
    }

    Serial.printf("|-- Sending buffer for reportId:%d:\n", report_id);
    print_buff(buffer, reqlen, "    ");
    
    return reqlen;
}

// Invoked when received SET_REPORT control request or
// received OUTPUT report (report_type == 0 )
// received FEATURE report (report_type == 3)
void set_report_callback(uint8_t report_id, hid_report_type_t report_type, 
                          uint8_t const* buffer, uint16_t bufsize) {
    board_leds_set(0x00FF00);
    
    Serial.printf("RECEIVED %s REPORT: ",
                 (report_type==3) ? "FEATURE" : (report_type==0) ? "OUTPUT" : "OTHER");
    Serial.printf(" report_id: %d",report_id);
    Serial.printf(" report_type: %d" ,report_type);
    Serial.print(" bufsize:"); Serial.print(bufsize);
    Serial.println();
    print_buff(buffer, bufsize, "  ");
    
    // echo back anything we received from host
    if( echoReports ) {
        Serial.printf("|-- Echoing report from OUT to IN, on reportId:%d\n", report_id); 
        memcpy(send_buff, buffer, bufsize); // save for GET_REPORT if feature report
        usb_hid.sendReport(report_id, buffer, bufsize);
    }
}

// clear output the input buffer
void drain_serial() { while( Serial.read() != -1 )  { } }

// print out a byte buffer as hex to Serial
void print_buff(const uint8_t* buf, int buflen, const char* line_start)
{
    int width = 16;
    Serial.print(line_start);
    for( int i=0; i<buflen; i++ ) {
        Serial.printf("%02X ", buf[i]);
        if (i % width == width-1 && i < buflen-1) {
            Serial.println(); Serial.print(line_start);
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
