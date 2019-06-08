/**
 * hidapitester.c -- Demonstrate HIDAPI via commandline
 * 
 * 2019, Tod E. Kurt / github.com/todbot
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <getopt.h> 

#include "hidapi.h"


#define MAX_STR 1024  // for manufacturer, product strings
#define MAX_BUF 1024  // for buf reads & writes


static void print_usage(char *myname)
{
    fprintf(stderr,
"Usage: \n"
"  %s <cmd> [options]\n"
"where <cmd> is one of:\n"
"  --list                      List connected HID devices \n"
"  --open <vid:pid>            Open device with VendorId/ProductId \n"
"  --open <vid:pid:usagePage:usage> Open but with usagePage & usage \n"
"  --open-path <pathstr>       Open device by path (as in --list) \n"
"  --close                     Close currently open device \n"
"  --send-out <datalist>       Send Ouput report to device \n"
"  --send-feature <datalist>   Send Feature report \n"
"  --read-feature [reportId]   Read Feature report (w/ report-id) \n"
"  --len <len>, -l <len>       Set length in bytes of report to send/read \n"
"  --timeout <msecs>           Timeout in millisecs to wait for input reads \n"
"  --quiet, -q                 Print out nothing except when reading data \n"
"", myname);
}

// local states for the "cmd" option variable
enum { 
    CMD_NONE = 0,
    CMD_VIDPID,
    CMD_USAGE,
    CMD_LIST,
    CMD_OPEN,
    CMD_OPEN_PATH,
    CMD_CLOSE,
    CMD_SEND_OUTPUT,
    CMD_SEND_FEATURE,
    CMD_READ_INPUT,
    CMD_READ_FEATURE,
};


bool msg_quiet = false;


/**
 * printf that can be shut up
 */
void msg(char* fmt, ...)
{
    va_list args;
    va_start(args,fmt);
    if( !msg_quiet ) {
        vprintf(fmt,args);
    }
    va_end(args);
}

/**
 * print out a buffer of len bufsize in decimal form
 */
void printbuf(char* buf, int bufsize)
{
    for (int i = 0; i < bufsize; i++) {
        printf("%d,", buf[i]);
    }
    printf("\n");
}
/**
 * same but for hex output
 */
void printbufhex(uint8_t* buf, int bufsize)
{
    for( int i=0;i<bufsize;i++) {
       printf("0x%0x, ",buf[i]);
    }
    printf("\n");
}

/**
 * Parse a comma-delimited 'string' containing numbers (dec,hex)
 * into a array'buffer' (of element size 'bufelem_size') and 
 * of max length 'buflen', using delimiter 'delim_str'
 * Returns number of bytes written
 */
int hexread(void* buffer, char* delim_str, char* string, int buflen, int bufelem_size)
{
    char    *s;
    int     pos = 0;
    if( string==NULL ) return -1;
    memset(buffer,0,buflen);  // bzero() not defined on Win32?
    while((s = strtok(string, delim_str)) != NULL && pos < buflen){
        string = NULL;
        switch(bufelem_size) { 
        case 1:
            ((uint8_t*)buffer)[pos++] = (uint8_t)strtol(s, NULL, 0); break;
        case 2:
            ((int*)buffer)[pos++] = (int)strtol(s, NULL, 0); break;
        }
    }
    return pos;
}

/**
 *
 */
int main(int argc, char* argv[])
{
    uint8_t buf[MAX_BUF];   // data buffer for send/recv
    wchar_t wstr[MAX_STR];  // string buffer for USB strings
    hid_device *dev = NULL; // HIDAPI device we will open
    int res;
    int i;
    int buflen = 0;         // length of buf in use
    int cmd = CMD_NONE;     // 
    int timeout_millis = 250;
    
    uint16_t vid = 0;        // productId 
    uint16_t pid = 0;        // vendorId
    uint16_t usage = 0;      // usage to search for, if any
    uint16_t usage_page = 0; // usagePage to search for, if any
    char devpath[MAX_STR];   // path to open, if filter by usage
        
    if(argc < 2){
        print_usage( "hidapitester" );
        exit(1);
    }
    
    struct option longoptions[] =
        {
         {"help", no_argument, 0, 'h'},
         //{"verbose",      optional_argument, 0,      'v'},
         {"quiet",        optional_argument, 0,      'q'},
         {"timeout",      required_argument, 0,      't'},
         {"length",       required_argument, 0,      'l'},
         {"vidpid",       required_argument, &cmd,   CMD_VIDPID},
         {"usage",        required_argument, &cmd,   CMD_USAGE},
         {"list",         no_argument,       &cmd,   CMD_LIST},
         {"open",         no_argument,       &cmd,   CMD_OPEN},
         {"open-path",    required_argument, &cmd,   CMD_OPEN_PATH},
         {"close",        no_argument,       &cmd,   CMD_CLOSE},
         {"send-output",  required_argument, &cmd,   CMD_SEND_OUTPUT},
         {"send-feature", required_argument, &cmd,   CMD_SEND_FEATURE},
         {"read-input",   required_argument, &cmd,   CMD_READ_INPUT},
         {"read-feature", required_argument, &cmd,   CMD_READ_FEATURE},
         {NULL,0,0,0}
        };
    
    bool done = false;
    int option_index = 0, opt;
    char* opt_str = "vht:l:q";
    while(!done) {
        opt = getopt_long(argc, argv, opt_str, longoptions, &option_index);
        if (opt==-1) done = true; // parsed all the args
        switch(opt) {
        case 0:  // long opts with no short opts
            
            if( cmd == CMD_LIST ) {
                struct hid_device_info *devs, *cur_dev;
                devs = hid_enumerate(vid, pid); // 0,0 = find all devices
                cur_dev = devs;
                while (cur_dev) {
                    if(  (!usage_page || cur_dev->usage_page == usage_page) &&
                         (!usage || cur_dev->usage == usage) ) {
                        printf("Device: '%ls' by '%ls'\n", cur_dev->product_string, cur_dev->manufacturer_string);
                        printf("  vid/pid: 0x%04hX / 0x%04hX\n",cur_dev->vendor_id, cur_dev->product_id);
                        printf("  usagePage / usage: 0x%x / 0x%x,   serial_number: %ls \n",
                               cur_dev->usage_page, cur_dev->usage, cur_dev->serial_number );
                        printf("  path: %s\n",cur_dev->path);
                    }
                    cur_dev = cur_dev->next;
                }
                hid_free_enumeration(devs);
            }
            else if( cmd == CMD_VIDPID ) {
                
                int wordbuf[10];
                int parsedlen = hexread(wordbuf, "/, ", optarg, sizeof(wordbuf), 2);
                vid = wordbuf[0]; pid = wordbuf[1];
                msg("Looking for vid/pid 0x%04X / 0x%04X\n",vid,pid);

            }
            else if( cmd == CMD_USAGE ) {
                
                int wordbuf[10]; // only need 2, extra as guard
                int parsedlen = hexread(wordbuf, "/, ", optarg, sizeof(wordbuf), 2);
                usage_page = wordbuf[0]; usage = wordbuf[1];
                msg("Looking for usagePage/usage 0x%04X / 0x%04X\n",usage_page,usage);
                
                struct hid_device_info *devs, *cur_dev;
                devs = hid_enumerate(vid, pid); // 0,0 = find all devices
                cur_dev = devs;
                while (cur_dev) {
                    if( usage_page && cur_dev->usage_page == usage_page && cur_dev->usage == usage) {
                        strncpy(devpath, cur_dev->path, MAX_STR); // save it!
                        printf("found:%s\n", devpath);
                    }
                    cur_dev = cur_dev->next;
                }
                hid_free_enumeration(devs);
            }
            else if( cmd == CMD_OPEN ) {
                if( devpath[0] ) {
                    msg("Opening device at %s\n",devpath);
                    dev = hid_open_path(devpath);
                }
                else {
                    msg("Opening device at 0x%04X / 0x%04X\n",vid,pid);
                    dev = hid_open(vid,pid,NULL);
                }
                if( dev==NULL ) {
                    msg("Error: could not open device.\n");
                }
            }
            else if( cmd == CMD_OPEN_PATH ) {

                msg("Opening device at %s\n",optarg);
                dev = hid_open_path(optarg);
                if( dev==NULL ) {
                    msg("Error: could not open device\n");
                }
            }
            else if( cmd == CMD_CLOSE ) {
                
                msg("Closing device\n");
                if(dev) {
                    hid_close(dev);
                    dev = NULL;
                }
            }
            else if( cmd == CMD_SEND_OUTPUT  ||
                     cmd == CMD_SEND_FEATURE ) {
                
                int parsedlen = hexread(buf, ", ", optarg, sizeof(buf), 1);
                if( parsedlen<1 ) { // no bytes or error
                    msg("Error: no bytes read as arg to --send...");
                    break;
                }
                buflen = (!buflen) ? parsedlen : buflen;
                //printbufhex(buf,parsedlen);  // debug
                
                if( !dev ) { 
                    msg("Error on send: no device opened.\n"); break;
                }
                if( cmd == CMD_SEND_OUTPUT ) { 
                    msg("Writing output report of %d-bytes...",buflen);
                    res = hid_write(dev, buf, buflen);
                }
                else {
                    msg("Writing %d-byte feature report...",buflen);
                    res = hid_send_feature_report(dev, buf, buflen);
                }
                msg("wrote %d bytes\n", res);
            }
            else if( cmd == CMD_READ_INPUT ) {
                if( !dev ) { 
                    msg("Error on read: no device opened.\n"); break;
                }
                
                memset(buf,0,buflen);
                msg("Reading %d-byte input report, %d msec timeout...", buflen,timeout_millis);
                res = hid_read_timeout(dev, buf, buflen, timeout_millis);
                msg("read %d bytes\n", res);
                msg("Report:\n");
                printbufhex(buf,buflen);

            }
            else if( cmd == CMD_READ_FEATURE ) {
                if( !dev ) { 
                    msg("Error on read: no device opened.\n"); break;
                }
                
                uint8_t report_id = strtol(optarg,NULL,10);
                memset(buf,0,buflen);
                buf[0] = report_id;
                msg("Reading %d-byte feature report, report_id %d...",buflen, report_id);
                res = hid_get_feature_report(dev, buf, buflen);
                msg("read %d bytes\n",res);
                msg("Report:\n");
                printbufhex(buf, buflen);
            }
            
            break; // case 0
        case 'h':
            print_usage("hidapitester");
            done = true;
            break;
        case 'l':
            buflen = strtol(optarg,NULL,10);
            msg("Set buflen to %d\n", buflen);
            break;
        case 't':
            timeout_millis = strtol(optarg,NULL,10);
            msg("Set timeout_millis to %d\n", buflen);
            break;
        case 'q':
            msg_quiet = true;
            break;
        } // switch(opt)
        
        if(dev) {
            // hid_close(dev);
        }
        res = hid_exit();
        
    } // while(!done)
    
} // main

