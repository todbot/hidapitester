#ifndef TOD_HID_DESCRIPTORS_H
#define TOD_HID_DESCRIPTORS_H

// possible modes the device can have
#define MODE_INOUT_NO_REPORTID       1
#define MODE_INOUT_WITH_REPORTIDS    2
#define MODE_FEATURE_NO_REPORTID     3
#define MODE_FEATURE_WITH_REPORTID   4
#define MODE_BLINK1                 98
#define MODE_TEENSY                 99

#define TRINKET_VID 0x239A
#define TRINKET_PID 0x801E
#define REPORT_COUNT_MAX 64



#if MODE == MODE_INOUT_NO_REPORTID
#define MODE_STR "INOUT_NO_REPORTID"
#define MODE_INFO "Responds to IN/OUT reports, no ReportIDs"
#define VID TRINKET_VID
#define PID TRINKET_PID
#define HID_DESC HID_DESC_INOUT_NO_REPORTID()

#elif MODE == MODE_INOUT_WITH_REPORTIDS
#define MODE_STR "INOUT_WITH_REPORTIDS"
#define MODE_INFO "Responds to IN/OUT reports, w/ ReportID #1 (8-byte) & ReportId #2 (64-byte)"
#define VID TRINKET_VID
#define PID TRINKET_PID
#define HID_DESC HID_DESC_INOUT_WITH_REPORTIDS()

#elif MODE == MODE_FEATURE_NO_REPORTID
#define MODE_STR "FEATURE_NO_REPORTID"
#define MODE_INFO "Responds to IN/OUT reports, no ReportIDs"
#define VID TRINKET_VID
#define PID TRINKET_PID
#define HID_DESC HID_DESC_FEATURE_NO_REPORTID(64)

#elif MODE == MODE_FEATURE_WITH_REPORTID
#define MODE_STR "FEATURE_WITH_REPORTID"
#define MODE_INFO "Responds to FEATURE reports, with reportId 1 (32-bytes)"
#define VID TRINKET_VID
#define PID TRINKET_PID
#define HID_DESC HID_DESC_FEATURE_WITH_REPORTID(32)

#elif MODE == MODE_BLINK1
#define MODE_STR "BLINK1"
#define MODE_INFO "Responds to FEATURE reports with reportID 1 (8-byte) & reportId 2 (60-byte)"
#define VID BLINK1_VID
#define PID BLINK1_PID
#define HID_DESC HID_DESC_BLINK1MK2()

#elif MODE == MODE_TEENSY
#define MODE_STR "TEENSY"
#define MODE_INFO "Responds to IN/OUT reports w/ no ReportIDs"
#define VID TEENSY_VID
#define PID TEENSY_PID
#define HID_DESC HID_DESC_TEENSY_RAWHID(64)

#else 
#error "undefined MODE"
#endif


#define TEENSY_VID 0x1212
#define TEENSY_PID 0x2222

#define RAWHID_USAGE_PAGE    0xFFAB  // recommended: 0xFF00 to 0xFFFF
#define RAWHID_USAGE         0x0200  // recommended: 0x0100 to 0xFFFF
#define RAWHID_TX_SIZE        64
#define RAWHID_RX_SIZE        64
#define LSB(n) (n & 255)
#define MSB(n) ((n >> 8) & 255)

#define HID_DESC_INOUT_NO_REPORTID(report_size,...) \
    0x06, LSB(RAWHID_USAGE_PAGE), MSB(RAWHID_USAGE_PAGE), \
    0x0A, LSB(RAWHID_USAGE), MSB(RAWHID_USAGE), \
    0xA1, 0x01,                     /* Collection 0x01 */ \
    0x75, 0x08,                     /* report size = 8 bits */ \
    0x15, 0x00,                     /* logical minimum = 0 */ \
    0x26, 0xFF, 0x00,               /* logical maximum = 255  */ \
    0x95, RAWHID_TX_SIZE,           /* report count */ \
    0x09, 0x01,                     /* usage */ \
    0x81, 0x02,                     /* Input (array) */ \
    0x95, RAWHID_RX_SIZE,           /* report count */ \
    0x09, 0x02,                     /* usage */ \
    0x91, 0x02,                     /* Output (array) */ \
    0xC0                            /* end collection */ \

#define HID_DESC_INOUT_WITH_REPORTIDS(report_size,...) \
    0x06, LSB(RAWHID_USAGE_PAGE), MSB(RAWHID_USAGE_PAGE), \
    0x0A, LSB(RAWHID_USAGE), MSB(RAWHID_USAGE), \
    0xA1, 0x01,                     /* Collection 0x01 */ \
    0x75, 0x08,                     /* report size = 8 bits */ \
    0x15, 0x00,                     /* logical minimum = 0 */ \
    0x26, 0xFF, 0x00,               /* logical maximum = 255  */ \
    0x85, 1,                        /*   REPORT_ID (1) */ \
    0x95, 64,                        /* report count */ \
    0x09, 0x01,                     /* usage */ \
    0x81, 0x02,                     /* Input (array) */ \
    0x95, 64,                        /* report count */ \
    0x09, 0x02,                     /* usage */ \
    0x91, 0x02,                     /* Output (array) */ \
    0x85, 1,                        /*   REPORT_ID (2) */ \
    0x95, 8,                       /* report count */ \
    0x09, 0x01,                     /* usage */ \
    0x81, 0x02,                     /* Input (array) */ \
    0x95, 8,                       /* report count */ \
    0x09, 0x02,                     /* usage */ \
    0x91, 0x02,                     /* Output (array) */ \
    0xC0                            /* end collection */ \

#define HID_DESC_FEATURE_NO_REPORTID(report_size, ...) \
    0x06, 0xAB, 0xFF,              /* Usage Page (Vendor Defined 0xFFAB) */ \
    0x0A, 0x00, 0x20,              /* Usage (0x0200) */ \
    0xA1, 0x01,                    /* COLLECTION (Application) */ \
    0x15, 0x00,                    /*   LOGICAL_MINIMUM (0) */ \
    0x26, 0xff, 0x00,              /*   LOGICAL_MAXIMUM (255)*/ \
    0x75, 0x08,                    /*   REPORT_SIZE (8) */ \
    0x95, report_size,             /*   REPORT_COUNT (64) */ \
    0x09, 0x00,                    /*   USAGE (Undefined) */ \
    0xb2, 0x02, 0x01,              /*   FEATURE (Data,Var,Abs,Buf) */ \
    0xc0                           /* END_COLLECTION */ \

#define HID_DESC_FEATURE_WITH_REPORTID(report_size, ...) \
    0x06, 0xAB, 0xFF,              /* Usage Page (Vendor Defined 0xFFAB) */ \
    0x0A, 0x00, 0x20,              /* Usage (0x0200) */ \
    0xA1, 0x01,                    /* COLLECTION (Application) */ \
    0x15, 0x00,                    /*   LOGICAL_MINIMUM (0) */ \
    0x26, 0xff, 0x00,              /*   LOGICAL_MAXIMUM (255)*/ \
    0x75, 0x08,                    /*   REPORT_SIZE (8) */ \
    0x85, 1,                       /*   REPORT_ID (1) */ \
    0x95, report_size,             /*   REPORT_COUNT (32) */ \
    0x09, 0x00,                    /*   USAGE (Undefined) */ \
    0xb2, 0x02, 0x01,              /*   FEATURE (Data,Var,Abs,Buf) */ \
    0xc0                           /* END_COLLECTION */ \


// .....

#define HID_DESC_TEENSY_RAWHID(report_size,...) \
    0x06, LSB(RAWHID_USAGE_PAGE), MSB(RAWHID_USAGE_PAGE), \
    0x0A, LSB(RAWHID_USAGE), MSB(RAWHID_USAGE), \
    0xA1, 0x01,                     /* Collection 0x01 */ \
    0x75, 0x08,                     /* report size = 8 bits */ \
    0x15, 0x00,                     /* logical minimum = 0 */ \
    0x26, 0xFF, 0x00,               /* logical maximum = 255  */ \
    0x95, RAWHID_TX_SIZE,           /* report count */ \
    0x09, 0x01,                     /* usage */ \
    0x81, 0x02,                     /* Input (array) */ \
    0x95, RAWHID_RX_SIZE,           /* report count */ \
    0x09, 0x02,                     /* usage */ \
    0x91, 0x02,                     /* Output (array) */ \
    0xC0                            /* end collection */ \

#define BLINK1_VID 0x27b8
#define BLINK1_PID 0x01ed

#define HID_DESC_BLINK1MK2(report_size, ...) \
    0x06, 0xAB, 0xFF, \
    0x0A, 0x00, 0x20, \
    0xA1, 0x01,                    /* COLLECTION (Application) */ \
    0x15, 0x00,                    /*   LOGICAL_MINIMUM (0) */ \
    0x26, 0xff, 0x00,              /*   LOGICAL_MAXIMUM (255)*/ \
    0x75, 0x08,                    /*   REPORT_SIZE (8) */ \
    0x85, 1,                       /*   REPORT_ID (1) */ \
    0x95, 8,                       /*   REPORT_COUNT (8) */ \
    0x09, 0x00,                    /*   USAGE (Undefined) */ \
    0xb2, 0x02, 0x01,              /*   FEATURE (Data,Var,Abs,Buf) */ \
    0x75, 0x08,                    /*   REPORT_SIZE (8) */ \
    0x85, 2,                       /*   REPORT_ID (1) */ \
    0x95, 60,                      /*   REPORT_COUNT (60) */ \
    0x09, 0x00,                    /*   USAGE (Undefined) */ \
    0xb2, 0x02, 0x01,              /*   FEATURE (Data,Var,Abs,Buf) */ \
    0xc0                           /* END_COLLECTION */ \


// HID Generic Input & Output
// - 1st parameter is report size (mandatory)
// - 2nd parameter is report id HID_REPORT_ID(n) (optional)
#define TOD_TUD_HID_REPORT_DESC_GENERIC_INOUT(report_size, ...) \
    HID_USAGE_PAGE_N ( HID_USAGE_PAGE_VENDOR, 2   ),\
    HID_USAGE        ( 0x01                       ),\
    HID_COLLECTION   ( HID_COLLECTION_APPLICATION ),\
      /* Report ID if any */\
      __VA_ARGS__ \
      /* Input */ \
      HID_USAGE       ( 0x02                                   ),\
      HID_LOGICAL_MIN ( 0x00                                   ),\
      HID_LOGICAL_MAX ( 0xff                                   ),\
      HID_REPORT_SIZE ( 8                                      ),\
      HID_REPORT_COUNT( report_size                            ),\
      HID_INPUT       ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ),\
      /* Output */ \
      HID_USAGE       ( 0x03                                    ),\
      HID_LOGICAL_MIN ( 0x00                                    ),\
      HID_LOGICAL_MAX ( 0xff                                    ),\
      HID_REPORT_SIZE ( 8                                       ),\
      HID_REPORT_COUNT( report_size                             ),\
      HID_OUTPUT      ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE  ),\
    HID_COLLECTION_END \



#endif
