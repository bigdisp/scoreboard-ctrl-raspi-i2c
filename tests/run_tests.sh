#!/usr/bin/env bash
# CLI tests for scoreboard_ctrl_i2c, run against the mocked wiringPi I2C library.
#
# Every test runs the binary with some arguments and compares
#   - the exit code
#   - the exact sequence of I2C calls recorded by the mock
#
# Tests marked with `xfail` document KNOWN BUGS: they describe the intended
# behaviour (as documented in the usage text) and are expected to fail today.
# They do not break the suite; if one starts passing it is reported as XPASS
# so the marker can be removed.
set -u

BIN=${1:-build/scoreboard_ctrl_i2c}
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

pass=0; fail=0; xfail=0; xpass=0

# run_case <name> <expected exit> <expected i2c log (\n separated)> -- args...
run_case() {
	local name=$1 exp_rc=$2 exp_log=$3; shift 4
	local log="$TMP/i2c.log"
	: > "$log"
	WIRINGPI_MOCK_LOG="$log" "$BIN" "$@" >"$TMP/stdout" 2>"$TMP/stderr"
	local rc=$?
	local got_log; got_log=$(cat "$log")
	if [[ $rc == "$exp_rc" && "$got_log" == "$(printf '%b' "$exp_log")" ]]; then
		return 0
	fi
	{
		echo "    args:      $*"
		echo "    exit:      expected $exp_rc, got $rc"
		echo "    i2c expected:"; printf '%b\n' "$exp_log" | sed 's/^/      /'
		echo "    i2c got:";      sed 's/^/      /' "$log"
	} > "$TMP/diag"
	return 1
}

check() {
	local name=$1
	if run_case "$@"; then
		echo "PASS  $name"; pass=$((pass+1))
	else
		echo "FAIL  $name"; cat "$TMP/diag"; fail=$((fail+1))
	fi
}

xfail() {
	local reason=$1; shift
	local name=$1
	if run_case "$@"; then
		echo "XPASS $name (known bug seems fixed - remove xfail marker)"; xpass=$((xpass+1))
	else
		echo "XFAIL $name  [known bug: $reason]"; xfail=$((xfail+1))
	fi
}

# --- argument handling ------------------------------------------------------

check "no arguments prints usage, no I2C traffic" 255 "" --
grep -q "Usage:" "$TMP/stderr" && grep -q "pwm value|on|off" "$TMP/stderr" \
	&& { echo "PASS  usage text content"; pass=$((pass+1)); } \
	|| { echo "FAIL  usage text content"; fail=$((fail+1)); }

WIRINGPI_MOCK_SETUP_FAIL=1 WIRINGPI_MOCK_LOG="$TMP/l" "$BIN" H1 5 2>"$TMP/stderr" >/dev/null
rc=$?
if [[ $rc == 254 ]] && grep -q "Error while connecting" "$TMP/stderr"; then
	echo "PASS  setup failure exit code/message"; pass=$((pass+1))
else
	echo "FAIL  setup failure exit code/message (rc=$rc)"; fail=$((fail+1))
fi

# --- digit / letter output --------------------------------------------------

check "digit is sent as ASCII byte to device 0x10" 0 "setup 0x10\nwrite 0x35" -- H1 5
check "letter is sent as ASCII byte"               0 "setup 0x10\nwrite 0x41" -- R10 A
check "only first char of a string is sent"        0 "setup 0x10\nwrite 0x61" -- I1 abc
check "missing value blanks the digit (space)"     0 "setup 0x10\nwrite 0x20" -- H1
check "dash is passed through"                     0 "setup 0x10\nwrite 0x2d" -- H10 -

# --- bootloader -------------------------------------------------------------
# Must be exactly the 3 bytes 0x10 0x4C 0xFB the firmware (scoreboard-atmega8-i2c)
# expects; WriteReg16 sends register, low byte, high byte.

check "bootloader: default address 0x10"   0 "setup 0x10\nwriteReg16 0x10 0xfb4c" -- bootloader
check "bootloader: hex address"            0 "setup 0x14\nwriteReg16 0x10 0xfb4c" -- bootloader 0x14
check "bootloader: decimal address"        0 "setup 0x11\nwriteReg16 0x10 0xfb4c" -- bootloader 17
check "bootloader: highest address 0x77"   0 "setup 0x77\nwriteReg16 0x10 0xfb4c" -- bootloader 0x77
check "bootloader: reserved address 0x0f"  1 "" -- bootloader 0x0f
check "bootloader: reserved address 0x78"  1 "" -- bootloader 0x78
check "bootloader: invalid address"        1 "" -- bootloader 0x1g
check "bootloader: empty address"          1 "" -- bootloader ""

WIRINGPI_MOCK_WRITE_FAIL=1 WIRINGPI_MOCK_LOG="$TMP/l" "$BIN" bootloader 2>"$TMP/stderr" >/dev/null
rc=$?
if [[ $rc == 3 ]] && grep -q "Error while sending bootloader command" "$TMP/stderr"; then
	echo "PASS  bootloader: write failure returns E_FAILURE_SEND_CMD"; pass=$((pass+1))
else
	echo "FAIL  bootloader: write failure returns E_FAILURE_SEND_CMD (rc=$rc)"; fail=$((fail+1))
fi

# --- pwm --------------------------------------------------------------------

check "'pwm' without value prints usage"           255 "setup 0x10" -- pwm

# Expected bytes follow the current firmware protocol (scoreboard-atmega8-i2c):
#   0x04 = PWM off, 0x05 = PWM on
# Two problems today:
#  - main() uses `if (strcmp(argv[2], "on"))`, i.e. the result is inverted
#    (fixed by bigdisp/scoreboard-ctrl-raspi-i2c#2):
#    "on" disables, "off" enables and numeric values never reach pwm().
#  - pwm_enable()/pwm_disable() send 0x03/0x02 from an older firmware protocol;
#    for the current firmware these set the colour of segment 1.
# pwm levels (pwm 0..10, pwm <on> <off>) are not tested: pwm() writes registers
# 0x00/0x01 and then 0x04, which the current firmware interprets as
# "red/blue segment 0", "green segment 0" and "PWM off" - this needs a decision
# how brightness should map onto the firmware's per-segment colour values.
xfail "old firmware opcodes (+ inverted strcmp on master)" "'pwm on' enables PWM (0x05)"   0 "setup 0x10\nwrite 0x05" -- pwm on
xfail "old firmware opcodes (+ inverted strcmp on master)" "'pwm off' disables PWM (0x04)" 0 "setup 0x10\nwrite 0x04" -- pwm off

echo
echo "Summary: $pass passed, $fail failed, $xfail expected failures (known bugs), $xpass unexpectedly passed"
[[ $fail == 0 ]]
