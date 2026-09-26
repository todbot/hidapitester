#
# Makefile for 'hidapitester'
# 2019 Tod E. Kurt, todbot.com
#

# overide this with something like `HIDAPI_DIR=../hidapi-libusb make`
HIDAPI_DIR ?= ../hidapi

# try to do some autodetecting
UNAME := $(shell uname -s)
ARCH := $(shell uname -m)

ifeq "$(UNAME)" "Darwin"
	OS=macos
	ARCH=universal
endif
ifeq "$(OS)" "Windows_NT"
	OS=windows
endif
ifeq "$(UNAME)" "Linux"
	OS=linux
endif
ifeq "$(UNAME)" "FreeBSD"
	OS=freebsd
endif


# construct version string from git tag
# allow overriding of GIT_TAG for automated builds
# If we have a file .git-tag (from source archive), read it.
# cat, not $(file <...): that needs make 4.0 and macOS ships 3.81.
ifneq ($(wildcard .git-tag),)
	GIT_TAG_RAW=$(strip $(shell cat .git-tag))
endif
# describe, not `git tag | tail -1`: that sorts lexically, so v0.6 beats v0.10.
# --match skips the legacy unprefixed tags (0.1 through 0.5).
GIT_TAG_RAW?=$(strip $(shell git describe --tags --abbrev=0 --match 'v*' 2>/dev/null))
# no git, or no vX.Y tag reachable
ifeq ($(GIT_TAG_RAW),)
	GIT_TAG_RAW:=v$(strip $(shell date -r . +'%Y%m%d'))
endif
GIT_TAG?="$(GIT_TAG_RAW)"
HIDAPITESTER_VERSION?="$(GIT_TAG)"


#############  Mac
ifeq "$(OS)" "macos"

CFLAGS+=-arch x86_64 -arch arm64
CFLAGS+=-I $(HIDAPI_DIR)/mac
LIBS=-framework IOKit -framework CoreFoundation -framework AppKit
OBJS=$(HIDAPI_DIR)/mac/hid.o
EXE=

endif

############# Windows
ifeq "$(OS)" "windows"

# deal with Windows not having 'cc'
ifeq (default,$(origin CC))
  CC = gcc
endif

LIBS += -lsetupapi -Wl,--enable-auto-import -static-libgcc -static-libstdc++
OBJS = $(HIDAPI_DIR)/windows/hid.o
EXE=.exe

endif

############ Linux (hidraw)
ifeq "$(OS)" "linux"

PKGS = libudev

ifneq ($(wildcard $(HIDAPI_DIR)),)
OBJS = $(HIDAPI_DIR)/linux/hid.o
else
PKGS += hidapi-hidraw hidapi-libusb
endif

CFLAGS += $(shell pkg-config --cflags $(PKGS))
LIBS = $(shell pkg-config --libs $(PKGS))
EXE=

endif

########### FreeBSD
ifeq "$(OS)" "freebsd"

CFLAGS += -I/usr/local/include
OBJS = $(HIDAPI_DIR)/libusb/hid.o
LIBS += -L/usr/local/lib -lusb -liconv -pthread
EXE=

endif

############# common

CFLAGS += -I $(HIDAPI_DIR)/hidapi
CFLAGS += -DHIDAPITESTER_VERSION=\"$(HIDAPITESTER_VERSION)\"
OBJS += hidapitester.o

all: hidapitester

$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


hidapitester: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o hidapitester$(EXE) $(LIBS)

clean:
	rm -f $(OBJS)
	rm -f hidapitester$(EXE)

test: hidapitester
	sh tests/test_nohardware.sh ./hidapitester$(EXE)

test-hw: hidapitester
	sh tests/test_hardware.sh ./hidapitester$(EXE)

package: hidapitester$(EXE)
	@echo "Packaging up hidapitester for '$(OS)-$(ARCH)'"
	zip hidapitester-$(OS)-$(ARCH).zip hidapitester$(EXE)

help:
	@echo "Targets:"
	@echo "  all        Build hidapitester (default)"
	@echo "  clean      Remove build artifacts"
	@echo "  test       Run no-hardware tests"
	@echo "  test-hw    Run hardware tests (requires hidtest_tinyusb device)"
	@echo "  package    Zip the binary for the current platform"

