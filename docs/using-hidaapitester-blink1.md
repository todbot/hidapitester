
# Using `hidapitester` with blink(1) USB LEDs

## Sending "FadeToRGB" commands

```
% hidapitester --vidpid 27b8:01ed -l 9 --open --send-feature 1,99,255,0,0
Opening device, vid/pid: 0x27B8/0x01ED
Writing 9-byte feature report...wrote 9 bytes:
 01 63 FF 00 02 00 00 00 00
Closing device
 ```

## Getting blink(1) firmware version

```
hidapitester --vidpid 27b8:01ed -l 9 --open --send-feature 1,118 --read-feature 1
Opening device, vid/pid: 0x27B8/0x01ED
Writing 9-byte feature report...wrote 9 bytes:
 01 76 00 00 00 00 00 00 00
Reading 9-byte feature report, report_id 1...read 8 bytes:
 00 76 00 32 34 00 00 00 00
Closing device
 ```
