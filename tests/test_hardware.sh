#!/bin/sh
# Hardware-in-the-loop tests for hidapitester.
# Requires a hidtest_tinyusb device flashed and connected.
#
# Usage: sh tests/test_hardware.sh [path/to/hidapitester] [PID]
#   PID defaults to EE32 (mode 0 — 32 bytes, no report ID)
#
# PID by mode:
#   EE32  mode 0 — IN/OUT 32 bytes, no report ID
#   EE33  mode 1 — IN/OUT 32 bytes, report ID 1
#   EEEE  mode 2 — IN/OUT 64 bytes (Teensy-style), no report ID
#   4444  mode 3 — FEATURE only, report ID 1 (8B) + ID 2 (60B)
#
# Switch modes by connecting to the device's serial port and sending: m <0-3>

BIN=${1:-./hidapitester}
PID=${2:-EE32}
VID=27b8
PASS=0
FAIL=0

check() {
    _desc="$1"; _exp="$2"; _pat="$3"; shift 3
    _out=$("$@" 2>&1); _act=$?
    if [ "$_act" -eq "$_exp" ] && echo "$_out" | grep -q "$_pat"; then
        PASS=$((PASS+1)); printf "PASS: %s\n" "$_desc"
    else
        FAIL=$((FAIL+1)); printf "FAIL: %s (exit=%d expected=%d)\n" "$_desc" "$_act" "$_exp"
        printf "  output: %s\n" "$_out"
    fi
}

printf "Running hardware tests with: %s  VID/PID: %s:%s\n\n" "$BIN" "$VID" "$PID"

# --- device discovery ---
check "device appears in --list"           0 "$VID"  "$BIN" --vidpid "$VID:$PID" --list
check "device listed with usagePage FFAB"  0 "FFAB"  "$BIN" --vidpid "$VID:$PID" --list-usages
check "--open by vid/pid succeeds"         0 "Device opened"  "$BIN" --vidpid "$VID:$PID" --open --close
check "--open by usagePage succeeds"       0 "Device opened"  "$BIN" --usagePage 0xFFAB --open --close

# --- report descriptor ---
check "get-report-descriptor returns data"  0 "Report Descriptor"  "$BIN" --vidpid "$VID:$PID" --open --get-report-descriptor

# --- mode-specific tests ---
case "$PID" in
EE32)
    printf "\nMode 0 tests (32-byte IN/OUT, no report ID)\n"
    check "send 32-byte output report"  0 "wrote 32 bytes"  "$BIN" --vidpid "$VID:$PID" -l 32 --open --send-output 1,2,3,4
    check "read input report"           0 "read"            "$BIN" --vidpid "$VID:$PID" -l 32 --open --read-input
    ;;
EE33)
    printf "\nMode 1 tests (32-byte IN/OUT, report ID 1)\n"
    check "send output with report ID 1"  0 "wrote 32 bytes"  "$BIN" --vidpid "$VID:$PID" -l 32 --open --send-output 1,2,3,4
    check "read input report"             0 "read"            "$BIN" --vidpid "$VID:$PID" -l 32 --open --read-input
    ;;
EEEE)
    printf "\nMode 2 tests (64-byte IN/OUT, no report ID)\n"
    check "send 64-byte output report"  0 "wrote 64 bytes"  "$BIN" --vidpid "$VID:$PID" -l 64 --open --send-output 1,2,3,4
    check "read input report"           0 "read"            "$BIN" --vidpid "$VID:$PID" -l 64 --open --read-input
    ;;
4444)
    printf "\nMode 3 tests (FEATURE reports only, report IDs 1 and 2)\n"
    check "send feature report"                  0 "wrote"    "$BIN" --vidpid "$VID:$PID" -l 9 --open --send-feature 1,99,44,22
    check "read feature report decimal ID"       0 "read"     "$BIN" --vidpid "$VID:$PID" -l 9 --open --read-feature 1
    check "read feature report hex ID 0x01"      0 "read"     "$BIN" --vidpid "$VID:$PID" -l 9 --open --read-feature 0x01
    check "read feature hex ID matches decimal"  0 "read"     "$BIN" --vidpid "$VID:$PID" -l 9 --open --read-feature 0x02
    # default GET_REPORT (echo off) always returns 'a','b','c','d' = 61 62 63 64
    check "GET_REPORT returns known default bytes"  0 "61 62 63 64"  "$BIN" --vidpid "$VID:$PID" -l 9 --open --read-feature 1
    ;;
*)
    printf "Unknown PID '%s' — skipping mode-specific tests\n" "$PID"
    ;;
esac

printf "\nResults: %d passed, %d failed\n" "$PASS" "$FAIL"
[ "$FAIL" -eq 0 ]
