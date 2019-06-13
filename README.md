# hidapitester

Simple command-line program to test HIDAPI

## Prebuilt binaries
- Mac OS X
- Linux
- Windows


### Testing
- The "TeensyRawHid" directory contains an Arduino sketch for Teensy.
It sends 64-byte Input reports every second.
It receives 64-byte Output reports, and prints them to Serial Monitor.


## Examples

```
./hidapitester --open 0x27b8/0x1ed -l 9 --send-feature 1,99,0,255,0  --read-feature 1 --close
Opening device at vid/pid 27b8/1ed
Set buflen to 9
Writing 9-byte feature report...wrote 9 bytes
Reading 9-byte feature report, report_id 1...read 8 bytes
Report:
0x0, 0x63, 0x0, 0xff, 0x0, 0x0, 0x0, 0x0, 0x0,
Closing device
```


## To build

```
git clone https://github.com/libusb/hidapi
git clone https://github.com/todbot/hidapitester
cd hidapitester
make
```

### Platform-specific requirements

On Mac:
- Install XCode

On Windows
- Install MSYS
- Build in a MinGW window

On Linux
- Install udev
```
sudo apt install libudev1 libudev-dev pkg-config python
```

## Random notes

* To do termianl screencast gifs (on Linux):

```
sudo apt install asciinema
sudo apt install gifsicle
npm install --global asciicast2gif
asciinema rec foo.cast
[do what you want]
asciicast2gif  foo.cast foo.gif
```

or just use `termtosvg`:
```
pip3 install --user termtosvg
termtosvg foo.svg
[do what you want, type 'exit' to stop shell]

```

