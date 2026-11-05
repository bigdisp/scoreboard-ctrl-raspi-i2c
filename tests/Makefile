# PC test suite for scoreboard_ctrl_i2c.
# Builds src/main.c against a mocked wiringPi I2C library and runs CLI tests.
# No Raspberry Pi / I2C hardware required.
#
#   make -C tests          # build + run all tests
#   make -C tests clean

CC      ?= gcc
CFLAGS  ?= -O0 -g -Wall -Wextra -Wno-unused-parameter
BUILD   := build
BIN     := $(BUILD)/scoreboard_ctrl_i2c

.PHONY: all test clean
all: test

$(BIN): ../src/main.c mock/wiringPiI2C_mock.c mock/wiringPiI2C.h
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -Imock -o $@ ../src/main.c mock/wiringPiI2C_mock.c

test: $(BIN)
	./run_tests.sh $(BIN)

clean:
	rm -rf $(BUILD)
