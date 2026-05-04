#!/bin/sh
# No-hardware tests for hidapitester.
# Usage: sh tests/test_nohardware.sh [path/to/hidapitester]

BIN=${1:-./hidapitester}
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

# accept exit 0 OR (exit 1 AND "No HID devices found") — for --list on machines with no USB
check_list() {
    _desc="$1"; shift
    _out=$("$@" 2>&1); _act=$?
    if [ "$_act" -eq 0 ] || { [ "$_act" -eq 1 ] && echo "$_out" | grep -q "No HID devices found"; }; then
        PASS=$((PASS+1)); printf "PASS: %s\n" "$_desc"
    else
        FAIL=$((FAIL+1)); printf "FAIL: %s (exit=%d)\n" "$_desc" "$_act"
        printf "  output: %s\n" "$_out"
    fi
}

printf "Running no-hardware tests with: %s\n\n" "$BIN"

# --- version / help ---
check "--version exits 0"                  0 "hidapitester version:" "$BIN" --version
check "--version prints hidapi version"    0 "hidapi version:"        "$BIN" --version
check "no args exits 1 with usage"         1 "Usage:"                 "$BIN"
check "-h prints usage"                    0 "Usage:"                 "$BIN" -h

# --- list (exit code varies by hardware presence) ---
check_list "--list does not crash"                      "$BIN" --list
check_list "--list-detail does not crash"               "$BIN" --list-detail
check_list "--list-usages does not crash"               "$BIN" --list-usages
check_list "--vidpid colon format parses"               "$BIN" --vidpid 27b8:01ed --list
check_list "--vidpid slash format parses"               "$BIN" --vidpid 27b8/01ed --list
check_list "--usagePage hex parses"                     "$BIN" --usagePage 0xFFAB --list
check_list "--usage hex parses"                         "$BIN" --usage 0x0200 --list

# --- error paths that don't require a device ---
check "--send-output without open prints error"  0 "Error on send: no device opened"  "$BIN" --send-output 1,2,3
check "--send-feature without open prints error" 0 "Error on send: no device opened"  "$BIN" --send-feature 1,2,3
check "--read-input without open prints error"   0 "Error on read: no device opened"  "$BIN" --read-input
check "--read-feature without open prints error" 0 "Error on read: no device opened"  "$BIN" --read-feature 1
check "--read-input-report without open"         0 "Error on read: no device opened"  "$BIN" --read-input-report 1

# --- hex report ID acceptance (the 0x fix) ---
check "--read-feature accepts hex 0x01"          0 "Error on read: no device opened"  "$BIN" --read-feature 0x01
check "--read-feature accepts hex 0x0a"          0 "Error on read: no device opened"  "$BIN" --read-feature 0x0a
check "--read-input-report accepts hex 0x01"     0 "Error on read: no device opened"  "$BIN" --read-input-report 0x01

# --- option validation ---
check "--width 0 prints error"  0 "print width must be greater than 0"  "$BIN" --width 0 --version

printf "\nResults: %d passed, %d failed\n" "$PASS" "$FAIL"
[ "$FAIL" -eq 0 ]
