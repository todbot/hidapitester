#ifndef TOD_HID_DESCRIPTORS_H
#define TOD_HID_DESCRIPTORS_H

#define TEENSY_VID 0x1212
#define TEENSY_PID 0x2222

#define RAWHID_USAGE_PAGE    0xFFAB  // recommended: 0xFF00 to 0xFFFF
#define RAWHID_USAGE         0x0200  // recommended: 0x0100 to 0xFFFF
#define RAWHID_TX_SIZE        64
#define RAWHID_RX_SIZE        64
#define LSB(n) (n & 255)
#define MSB(n) ((n >> 8) & 255)

#define HID_DESC_INOUT_NO_REPORTID(report_size) \
    0x06, LSB(RAWHID_USAGE_PAGE), MSB(RAWHID_USAGE_PAGE), \
    0x0A, LSB(RAWHID_USAGE), MSB(RAWHID_USAGE), \
    0xA1, 0x01,                     /* Collection 0x01 */ \
    0x75, 0x08,                     /* report size = 8 bits */ \
    0x15, 0x00,                     /* logical minimum = 0 */ \
    0x26, 0xFF, 0x00,               /* logical maximum = 255  */ \
    0x95, report_size ,          /* report count */ \
    0x09, 0x01,                     /* usage */ \
    0x81, 0x02,                     /* Input (array) */ \
    0x95, report_size,           /* report count */ \
    0x09, 0x02,                     /* usage */ \
    0x91, 0x02,                     /* Output (array) */ \
    0xC0                            /* end collection */ \

#define HID_DESC_INOUT_WITH_REPORTID(report_size) \
    0x06, LSB(RAWHID_USAGE_PAGE), MSB(RAWHID_USAGE_PAGE), \
    0x0A, LSB(RAWHID_USAGE), MSB(RAWHID_USAGE), \
    0xA1, 0x01,                     /* Collection 0x01 */ \
    0x75, 0x08,                     /* report size = 8 bits */ \
    0x15, 0x00,                     /* logical minimum = 0 */ \
    0x26, 0xFF, 0x00,               /* logical maximum = 255  */ \
    0x85, 1,                        /*   REPORT_ID (1) */ \
    0x95, report_size,          /* report count */ \
    0x09, 0x01,                     /* usage */ \
    0x81, 0x02,                     /* Input (array) */ \
    0x95, report_size,          /* report count */ \
    0x09, 0x02,                     /* usage */ \
    0x91, 0x02,                     /* Output (array) */ \
    0xC0                            /* end collection */ \

#define HID_DESC_FEATURE_NO_REPORTID(report_size) \
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

#define HID_DESC_FEATURE_WITH_REPORTID(report_size) \
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

#define HID_DESC_TEENSY_RAWHID()                          \
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

#define HID_DESC_BLINK1MK2() \
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



#endif
