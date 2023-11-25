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

// normally this is obtained from git tags and filled out by the Makefile
#ifndef HIDAPITESTER_VERSION
#define HIDAPITESTER_VERSION "v0.0"
#endif

static void print_usage(char *myname)
{
    fprintf(stderr,
"Usage: \n"
"  %s <cmd> [options]\n"
"where <cmd> is one of:\n"
"  --vidpid <vid/pid>          Filter by vendorId/productId (comma/slash delim)\n"
"  --usagePage <number>        Filter by usagePage \n"
"  --usage <number>            Filter by usage \n"
"  --serial <string>           Filter by serial number \n"
"  --list                      List HID devices (by filters)\n"
"  --list-usages               List HID devices w/ usages (by filters)\n"
"  --list-detail               List HID devices w/ details (by filters)\n"
"  --open                      Open device with previously selected filters\n"
"  --open-path <pathstr>       Open device by path (as in --list-detail) \n"
"  --close                     Close currently open device \n"
"  --get-report-descriptor     Get the report descriptor\n"
"  --send-feature <datalist>   Send Feature report (1st byte reportId, if used)\n"
"  --read-feature <reportId>   Read Feature report (w/ reportId, 0 if unused) \n"
"  --send-output <datalist>    Send Ouput report to device \n"
"  --read-input                Read Input reports \n"
"  --read-input-forever        Read Input reports in a loop forever \n"
"  --read-input-report <reportId>  Read Input report from specific reportId \n"
"  --length <len>, -l <len>    Set buffer length in bytes of report to send/read\n"
"  --timeout <msecs>           Timeout in millisecs to wait for input reads \n"
"  --base <base>, -b <base>    Set decimal or hex buffer print mode\n"
"  --quiet, -q                 Print out nothing except when reading data \n"
"  --verbose, -v               Print out extra information\n"
"  --version                   Print out hidapitester and hidapi version\n"
"\n"
"Notes: \n"
" . Commands are executed in order. \n"
" . --vidpid, --usage, --usagePage, --serial act as filters to --open and --list \n"
"\n"
"Examples: \n"
". List all devices \n"
"   hidapitester --list \n"
". List details of all devices w/ vendorId 0x2341 \n"
"   hidapitester --vidpid 2341 --list-detail \n"
". Open vid/pid xxxx:yyyy, get report descriptor\n"
"   hidapitester --vidpid xxxx:yyyy --open --get-report-descriptor \n"
". Open device with usagePage 0xFFAB, send Feature report on reportId 1\n"
"   hidapitester -l 9 --usagePage 0xFFAB --open --send-feature 1,99,44,22 \n"
". Open vid/pid xxxx:yyyy, send 64-byte Output report, read 64-byte Input report\n"
"   hidapitester --vidpid xxxx:yyyy -l 64 --open --send-output 1,2,3 --read-input \n"
". Read Input report continuously with 1500 msec timeout \n"
"   hidapitester --vidpid xxxx:yyyy -l 64 -t 1500 --open --read-input-forever\n"
". Send FadeToRGB #FF00FF command to blink(1)\n"
"   hidapitester --vidpid 27b8:01ed -l 9 --open --send-feature 1,99,255,0,255\n"
"\n"
""

"", myname);
}

// local states for the "cmd" option variable
enum {
    CMD_NONE = 0,
    CMD_VERSION,
    CMD_VIDPID,
    CMD_USAGE,
    CMD_USAGEPAGE,
    CMD_SERIALNUMBER,
    CMD_LIST,
    CMD_LIST_USAGES,
    CMD_LIST_DETAIL,
    CMD_OPEN,
    CMD_OPEN_PATH,
    CMD_CLOSE,
    CMD_GET_REPORT_DESCRIPTOR,
    CMD_SEND_OUTPUT,
    CMD_SEND_FEATURE,
    CMD_READ_INPUT,
    CMD_READ_FEATURE,
    CMD_READ_INPUT_FOREVER,
    CMD_READ_INPUT_REPORT,
};

bool msg_quiet = false;
bool msg_verbose = false;

int print_base = 16; // 16 or 10, hex or decimal
int print_width = 32; // how many characters per line
/**
 * printf that can be shut up
 */
void msg(char* fmt, ...)
{
    va_list args;
    va_start(args,fmt);
    if(!msg_quiet) { vprintf(fmt,args); }
    va_end(args);
}
/**
 * printf that is wordy
 */
void msginfo(char* fmt, ...)
{
    va_list args;
    va_start(args,fmt);
    if(msg_verbose) { vprintf(fmt,args); }
    va_end(args);
}

/**
 * print out a buffer of len bufsize in decimal or hex form
 */
void printbuf(uint8_t* buf, int bufsize, int base, int width)
{
    for( int i=0 ; i<bufsize; i++) {
        if( base==10 ) {
            printf(" %3d", buf[i]);
        } else if( base==16 ) {
            printf(" %02X", buf[i] );
        }
       // if (i % 16 == 15 && i < bufsize-1) printf("\n");
       if (i % width == width-1 && i < bufsize-1) printf("\n");
    }
    printf("\n");
}

/**
 * Parse a comma-delimited 'string' containing numbers (dec,hex)
 * into a array'buffer' (of element size 'bufelem_size') and
 * of max length 'buflen', using delimiter 'delim_str'
 * Returns number of bytes written
 */
int str2buf(void* buffer, char* delim_str, char* string, int buflen, int bufelem_size)
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
    hid_device *dev = NULL; // HIDAPI device we will open
    int res;
    int i;
    int buflen = 64;        // length of buf in use
    int cmd = CMD_NONE;     //
    int timeout_millis = 250;

    uint16_t vid = 0;        // productId
    uint16_t pid = 0;        // vendorId
    uint16_t usage_page = 0; // usagePage to search for, if any
    uint16_t usage = 0;      // usage to search for, if any
    wchar_t serial_wstr[MAX_STR/4] = {L'\0'}; // serial number string rto search for, if any
    char devpath[MAX_STR];   // path to open, if filter by usage
    int descriptorMaxLen = HID_API_MAX_REPORT_DESCRIPTOR_SIZE;
    unsigned char descriptorBuf[descriptorMaxLen];

    setbuf(stdout, NULL);  // turn off buffering of stdout

    if(argc < 2){
        print_usage( "hidapitester" );
        exit(1);
    }

    struct option longoptions[] =
        {
         {"help", no_argument, 0, 'h'},
         {"version",      no_argument, &cmd,         CMD_VERSION},
         {"verbose",      no_argument, 0,            'v'},
         {"quiet",        optional_argument, 0,      'q'},
         {"timeout",      required_argument, 0,      't'},
         {"length",       required_argument, 0,      'l'},
         {"buflen",       required_argument, 0,      'l'},
         {"base",         required_argument, 0,      'b'},
         {"vidpid",       required_argument, &cmd,   CMD_VIDPID},
         {"usage",        required_argument, &cmd,   CMD_USAGE},
         {"usagePage",    required_argument, &cmd,   CMD_USAGEPAGE},
         {"serial",       required_argument, &cmd,   CMD_SERIALNUMBER},
         {"list",         no_argument,       &cmd,   CMD_LIST},
         {"list-usages",  no_argument,       &cmd,   CMD_LIST_USAGES},
         {"list-detail",  no_argument,       &cmd,   CMD_LIST_DETAIL},
         {"open",         no_argument,       &cmd,   CMD_OPEN},
         {"open-path",    required_argument, &cmd,   CMD_OPEN_PATH},
         {"close",        no_argument,       &cmd,   CMD_CLOSE},
         {"send-output",  required_argument, &cmd,   CMD_SEND_OUTPUT},
         {"send-out",     required_argument, &cmd,   CMD_SEND_OUTPUT},
         {"send-feature", required_argument, &cmd,   CMD_SEND_FEATURE},
         {"read-input",   no_argument,       &cmd,   CMD_READ_INPUT},
         {"read-in",      no_argument,       &cmd,   CMD_READ_INPUT},
         {"read-input-report", required_argument, &cmd,  CMD_READ_INPUT_REPORT},
         {"read-feature", required_argument, &cmd,   CMD_READ_FEATURE},
         {"read-input-forever",  optional_argument, &cmd,   CMD_READ_INPUT_FOREVER},
         {"get-report-descriptor", no_argument, &cmd, CMD_GET_REPORT_DESCRIPTOR},
         {NULL,0,0,0}
        };
    char* shortopts = "vht:l:qb:";

    bool done = false;
    int option_index = 0, opt;
    while(!done) {
        memset(buf,0, MAX_BUF);   // reset buffers
        memset(devpath,0,MAX_STR);

        opt = getopt_long(argc, argv, shortopts, longoptions, &option_index);
        if (opt==-1) done = true; // parsed all the args
        switch(opt) {
        case 0:                   // long opts with no short opts

            if( cmd == CMD_VIDPID ) {

                if( sscanf(optarg, "%4hx/%4hx", &vid,&pid) !=2 ) {  // match "23FE/AB12"
                    if( !sscanf(optarg, "%4hx:%4hx", &vid,&pid) ) { // match "23FE:AB12"
                        // else try parsing standard dec/hex values
                        int wordbuf[4]; // a little extra space
                        int parsedlen = str2buf(wordbuf, ":/, ", optarg, sizeof(wordbuf), 2);
                        vid = wordbuf[0]; pid = wordbuf[1];
                    }
                }
                msginfo("Looking for vid/pid 0x%04X / 0x%04X  (%d / %d)\n",vid,pid,vid,pid);
            }
            else if( cmd == CMD_USAGEPAGE ) {

                if( (usage_page = strtol(optarg,NULL,0)) == 0 ) { // if bad parse
                    sscanf(optarg, "%4hx", &usage_page ); // try bare "ABCD"
                }

                msginfo("Set usagePage to 0x%04hX (%d)\n", usage_page,usage_page);
            }
            else if( cmd == CMD_USAGE ) {

                if( (usage = strtol(optarg,NULL,0)) == 0 ) { // if bad parse
                    sscanf(optarg, "%4hx", &usage ); // try bare "ABCD"
                }
                msginfo("Set usage to 0x%04hX (%d)\n", usage,usage);
            }
            else if( cmd == CMD_SERIALNUMBER ) {

                swprintf( serial_wstr, sizeof(serial_wstr), L"%s", optarg); // convert to wchar_t*
            }
            else if( cmd == CMD_LIST ||
                     cmd == CMD_LIST_USAGES ||
                     cmd == CMD_LIST_DETAIL ) {

                struct hid_device_info *devs, *cur_dev;
                devs = hid_enumerate(vid,pid); // 0,0 = find all devices
                cur_dev = devs;
                while (cur_dev) {
                    if( (!usage_page || cur_dev->usage_page == usage_page) &&
                        (!usage || cur_dev->usage == usage) &&
                        (serial_wstr[0]==L'\0' || wcscmp(cur_dev->serial_number, serial_wstr)==0) ) {
                        if( cmd == CMD_LIST_USAGES ) {
                            printf("%04X/%04X / %04hX/%04hX  %ls - %ls\n",
                                   cur_dev->vendor_id, cur_dev->product_id,
                                   cur_dev->usage_page, cur_dev->usage ,
                                   cur_dev->manufacturer_string, cur_dev->product_string );
                        }
                        else {
                            printf("%04X/%04X: %ls - %ls\n",
                                   cur_dev->vendor_id, cur_dev->product_id,
                                   cur_dev->manufacturer_string, cur_dev->product_string );
                        }

                        if( cmd == CMD_LIST_DETAIL ) {
                            printf("  vendorId:      0x%04hX\n", cur_dev->vendor_id);
                            printf("  productId:     0x%04hX\n", cur_dev->product_id);
                            printf("  usagePage:     0x%04hX\n", cur_dev->usage_page);
                            printf("  usage:         0x%04hX\n", cur_dev->usage );
                            printf("  serial_number: %ls \n", cur_dev->serial_number);
                            printf("  interface:     %d \n", cur_dev->interface_number);
                            printf("  path: %s\n",cur_dev->path);
                            printf("\n");
                        }
                    }
                    cur_dev = cur_dev->next;
                }
                hid_free_enumeration(devs);
            }
            else if( cmd == CMD_OPEN ) {
                if( vid && pid && !usage_page && !usage ) {
                    msg("Opening device, vid/pid: 0x%04X/0x%04X\n",vid,pid);
                    dev = hid_open(vid,pid,NULL);
                }
                else {
                    msg("Opening device, vid/pid:0x%04X/0x%04X, usagePage/usage: %X/%X\n",
                        vid,pid,usage_page,usage);

                    struct hid_device_info *devs, *cur_dev;
                    devs = hid_enumerate(vid, pid); // 0,0 = find all devices
                    cur_dev = devs;
                    while (cur_dev) {
                        if( (!vid || cur_dev->vendor_id == vid) &&
                            (!pid || cur_dev->product_id == pid) &&
                            (!usage_page || cur_dev->usage_page == usage_page) &&
                            (!usage || cur_dev->usage == usage) &&
                            (serial_wstr[0]==L'\0' || wcscmp(cur_dev->serial_number, serial_wstr)==0) ) {
                            strncpy(devpath, cur_dev->path, MAX_STR); // save it!
                        }
                        cur_dev = cur_dev->next;
                    }
                    hid_free_enumeration(devs);

                    if( devpath[0] ) {
                        dev = hid_open_path(devpath);
                        if( dev==NULL ) {
                            msg("Error: could not open device\n");
                        }
                        else {
                            msg("Device opened\n");
                        }
                    }
                    else {
                        msg("Error: no matching devices\n");
                    }
                }
            }
            else if( cmd == CMD_OPEN_PATH ) {

                msg("Opening device. path: %s\n",optarg);
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
            else if( cmd == CMD_GET_REPORT_DESCRIPTOR ) {
                if( !dev ) {
                    msg("Error on send: no device opened.\n"); break;
                }
                printf("Report Descriptor:\n");
                int descriptorLen = hid_get_report_descriptor(dev, descriptorBuf, descriptorMaxLen);
                printbuf(descriptorBuf, descriptorLen, print_base, print_width);
            }
            else if( cmd == CMD_SEND_OUTPUT  ||
                     cmd == CMD_SEND_FEATURE ) {

                int parsedlen = str2buf(buf, ", ", optarg, sizeof(buf), 1);
                if( parsedlen<1 ) { // no bytes or error
                    msg("Error: no bytes read as arg to --send...");
                    break;
                }
                buflen = (!buflen) ? parsedlen : buflen;

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
                msg("wrote %d bytes:\n", res);
                if(!msg_quiet) { printbuf(buf,buflen, print_base, print_width); }
            }
            else if( cmd == CMD_READ_INPUT ||
                     cmd == CMD_READ_INPUT_FOREVER ) {

                if( !dev ) {
                    msg("Error on read: no device opened.\n"); break;
                }
                if( !buflen) {
                    msg("Error on read: buffer length is 0. Use --len to specify.\n"); break;
                }
                do {
                    msg("Reading up to %d-byte input report, %d msec timeout...",
                      buflen, timeout_millis);
                    res = hid_read_timeout(dev, buf, buflen, timeout_millis);
                    msg("read %d bytes:\n", res);
                    if( res > 0 ) {
                        printbuf(buf,buflen, print_base, print_width);
                        memset(buf,0,buflen);  // clear it out
                    }
                    else if( res == -1 )  { // removed device
                        cmd = CMD_CLOSE;
                        break;
                    }
                } while( cmd == CMD_READ_INPUT_FOREVER );
            }
            else if( cmd == CMD_READ_INPUT_REPORT ) {
                if( !dev ) {
                    msg("Error on read: no device opened.\n"); break;
                }
                if( !buflen) {
                    msg("Error on read: buffer length is 0. Use --len to specify.\n");
                    break;
                }
                uint8_t report_id = (optarg) ? strtol(optarg,NULL,10) : 0;
                buf[0] = report_id;
                msg("Reading %d-byte input report using hid_get_input_report, report_id %d...",
                    buflen, report_id);
                res = hid_get_input_report(dev, buf, buflen);
                msg("read %d bytes:\n",res);
                printbuf(buf, buflen, print_base, print_width);
            }
            else if( cmd == CMD_READ_FEATURE ) {

                if( !dev ) {
                    msg("Error on read: no device opened.\n"); break;
                }
                if( !buflen) {
                    msg("Error on read: buffer length is 0. Use --len to specify.\n");
                    break;
                }
                uint8_t report_id = (optarg) ? strtol(optarg,NULL,10) : 0;
                buf[0] = report_id;
                msg("Reading %d-byte feature report, report_id %d...",buflen, report_id);
                res = hid_get_feature_report(dev, buf, buflen);
                msg("read %d bytes:\n",res);
                printbuf(buf, buflen, print_base, print_width);
            }
            else if( cmd == CMD_VERSION ) {
                printf("hidapitester version: %s\n", HIDAPITESTER_VERSION);
                printf("hidapi version: %d.%d.%d\n",
                       HID_API_VERSION_MAJOR, HID_API_VERSION_MINOR, HID_API_VERSION_PATCH);
            }

            break; // case 0 (longopts without shortops)
        case 'h':
            print_usage("hidapitester");
            break;
        case 'l':
            buflen = strtol(optarg,NULL,10);
            msginfo("Set buflen to %d\n", buflen);
            break;
        case 't':
            timeout_millis = strtol(optarg,NULL,10);
            msginfo("Set timeout_millis to %d\n", timeout_millis);
            break;
        case 'b':
            print_base = strtol(optarg,NULL,10);
            msginfo("Set print_base to %d\n", print_base);
            break;
        case 'q':
            msg_quiet = true;
            break;
        case 'v':
            msg_verbose = true;
            break;
        } // switch(opt)


    } // while(!done)

    if(dev) {
        msg("Closing device\n");
        hid_close(dev);
    }
    res = hid_exit();

} // main
