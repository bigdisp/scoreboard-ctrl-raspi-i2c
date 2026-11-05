# Builds and runs the unit tests of the control program.
# No hardware and no wiringPi installation is required: the i2c bus is
# simulated by the stubs in test_main.c.

CC ?= gcc
CFLAGS += -Wall -Wextra -g -I.

all: test

test_main: test_main.c ../src/main.c wiringPiI2C.h
	$(CC) $(CFLAGS) -o $@ test_main.c

test: test_main
	./test_main

clean:
	rm -f test_main

.PHONY: all test clean
