# hidapitester

Simple command-line program to exercise HIDAPI

[![Build Status linux](https://github.com/todbot/hidapitester/actions/workflows/linux.yml/badge.svg)](https://github.com/todbot/hidapitester/actions?query=workflow%3Alinux)
[![Build Status macos](https://github.com/todbot/hidapitester/actions/workflows/macos.yml/badge.svg)](https://github.com/todbot/hidapitester/actions?query=workflow%3Amacos)
[![Build Status windows](https://github.com/todbot/hidapitester/actions/workflows/windows.yml/badge.svg)](https://github.com/todbot/hidapitester/actions?query=workflow%3Awindows)


The goal of the `hidapitester` program is to provide a simple,
low-dependency command-line tool to test out every API call in
[hidapi](https://github.com/libusb/hidapi).
Default builds are fully-static with no requirements on a system-installed `hidapi`.

<img src="./docs/screencast1a.gif" width="500">

## Table of Contents

  * [Prebuilt binaries](#prebuilt-binaries)
  * [Usage](#usage)
     * [Listing Devices](#listing-devices)
     * [Opening Devices](#opening-devices)
     * [Reading and Writing Reports](#reading-and-writing-reports)
  * [Examples](#examples)
     * [Test Hardware](#test-hardware)
  * [Compiling](#compiling)
     * [Platform-specific requirements](#platform-specific-requirements)
        * [Mac](#mac)
        * [Windows](#windows)
        * [Linux](#linux)

## Prebuilt binaries

See the [hidapitester releases page](https://github.com/todbot/hidapitester/releases)
for builds for:

- Mac OS X (Intel and M1)
- Linux (Ubuntu x64 and Raspberry Pi)
- Windows 64-bit

## Usage

`hidapitester` works by parsing a list of arguments as commands it executes in order.
Those commands are:

```text
  --vidpid <vid/pid>          Filter by vendorId/productId (comma/slash delim)
  --usagePage <number>        Filter by usagePage
  --usage <number>            Filter by usage
  --list                      List HID devices (by filters)
  --list-usages               List HID devices w/ usages (by filters)
  --list-detail               List HID devices w/ details (by filters)
  --open                      Open device with previously selected filters
  --open-path <pathstr>       Open device by path (as in --list-detail)
  --close                     Close currently open device
  --get-report-descriptor     Get the report descriptor
  --send-feature <datalist>   Send Feature report (1st byte reportId, if used)
  --read-feature <reportId>   Read Feature report (w/ reportId, 0 if unused)
  --send-output <datalist>    Send Ouput report to device
  --read-input                Read Input reports
  --read-input-forever        Read Input reports in a loop forever
  --read-input-report <reportId>  Read Input report from specific reportId
  --length <len>, -l <len>    Set buffer length in bytes of report to send/read
  --timeout <msecs>           Timeout in millisecs to wait for input reads
  --base <base>, -b <base>    Set decimal or hex buffer print mode
  --quiet, -q                 Print out nothing except when reading data
  --verbose, -v               Print out extra information
```

### Listing Devices

* `--list` shows devices similar to `lsusb`
* `--list-usages` includes usagePage and usage attributes
* `--list-detail` shows all available information,
including usagePage, usage, path, and more
* Use `--vidpid`, `--usagePage`, or `--usage` to filter the output

* The `--vidpid` commmand allows full or partial specification of the
Vendor Id and Product Id.  These are all valid:

```text
  --vidpid 16C0:FFAB  # specify both vid 0x16C0 and pid 0xFFAB
  --vidpid 16C0       # just specify the vid
  --vidpid 0:FFAB     # just specify the pid
  --vidpid 16C0:FFAB  # use colon instead of slash
```

### Opening Devices

You must `--open` before you can `--read-input`. You can also `--read-input`
multiple times, or `--open` one device, `--close` it, and `--open` another.

The `--open` command will take whichever of VID, PID, usagePage, and usage are
specified.  So these are valid:

```text
hidapitester --vidpid 16C0 --usagePage FFAB --open      # specify vid and usagePage
hidapitester --usage FFAB --open                        # specify only usagePage
hidapitester --0/0486  --open                           # specify only pid
hidapitester --vidpid 16C0/486 --usagePage FFAB --open  # specify vid,pid,usagePage
```

### Reading and Writing Reports

Get the report descriptor with `--get-report-descriptor`.

Send Output reports to devices with `--send-output`. The argument to the command
is the data to send: `--send-output 1,2,0xff,30,40,0x50`.
If using reportIds, the first byte is the reportId.
If not using reportIds, the first byte should be `0`.
The length of the actual report is set by `--length <num>`.

Thus to send a 16-byte report on reportId 3 with only the 1st byte set to "42":

```text
hidapitester [...] --length 16 --send-output 3,42
```

Send Feature reports the same way with `--send-feature`.

Read Input reports from device with `--read-input`.  If using reportIds,
use `--read-input-report n` where the `n` argument is the reportId number:
e.g.  `--read-input 1`.  The length to read is specified by the `--length` argument.
If using reportIds, this length should be one more than the buffer to read
(e.g. if the report is 16-bytes, length is 17).

So to read a 16-byte report on reportId 3:

```text
hidapitester [...] --length 17 --read-input-report 3
```

## Examples

Get version info from a blink(1):

```text
hidapitester --vidpid 0x27b8/0x1ed --open --length 9 --send-feature 1,99,0,255,0  --read-feature 1 --close
Opening device at vid/pid 27b8/1ed
Set buflen to 9
Writing 9-byte feature report...wrote 9 bytes
Reading 9-byte feature report, report_id 1...read 8 bytes
Report:
0x0, 0x63, 0x0, 0xff, 0x0, 0x0, 0x0, 0x0, 0x0,
Closing device
```

Send data to/from "TeensyRawHid" sketch:

```text
hidapitester --vidpid 16C0 --usagePage 0xFFAB --open --send-output 0x4f,33,22,0xff  --read-input
Opening device, vid/pid:0x16C0/0x0000, usagePage/usage: FFAB/0
Device opened
Writing output report of 64-bytes...wrote 64 bytes:
 4F 21 16 FF 00 00 00 00 00 00 00 00 00 00 00 00
 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
Reading 64-byte input report, 250 msec timeout...read 64 bytes:
 AB CD 01 67 01 6F 01 93 01 94 01 A6 01 AA 01 67
 01 82 01 7D 01 79 01 18 01 0B 00 00 00 00 00 00
 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
 00 00 00 00 00 00 00 00 00 00 00 00 00 00 09 91
Closing device
```

### Test Hardware

- The "TeensyRawHid" directory contains an Arduino sketch for
[Teensy microcontrollers](https://www.pjrc.com/teensy/).
The sketch sends 64-byte Input reports every second, with no reportId.
The sketch receives 64-byte Output reports, and prints them to Serial Monitor.

- The "ProMicroRawHID" directory contains an Arduino sketch for any microcontroller
board supported by NicoHood's [HID Project](https://github.com/NicoHood/HID)
This sketch sends a 64-byte Input report every 2 seconds, with no reportId.
The sketch recives 64-byte Output or Feature reports, and prints them
to Serial Monitor

## Compiling

Building `hidapitester` is done via a very simple Makefile.

```text
git clone https://github.com/libusb/hidapi
git clone https://github.com/todbot/hidapitester
cd hidapitester
make
```

`hidapitester` will use a copy of `hidapi` located next to it in the directory hierarchy.
If you install `hidapi` in a different directory, you can set the Makefile
variable `HIDAPI_DIR` before invoking `make`:

```text
# hidapi is in dir 'hidapi-libusb-test'
cd hidapitester
HIDAPI_DIR=../hidapi-libusb-test make clean
HIDAPI_DIR=../hidapi-libusb-test make
./hidapitester --list
```

### Platform-specific requirements

#### Mac

- Install XCode
- Specifically, Command-line Tools

```text
sudo xcode-select --install
```

#### Windows

- Install MSYS2
- Build in a MinGW / MSYS2 window

#### Linux

- Install udev, pkg-config

```text
sudo apt install libudev1 libudev-dev pkg-config
```
