/*
 * Host-side mock of wiringPi's I2C API. See wiringPiI2C.h in this folder.
 */
#include "wiringPiI2C.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define MOCK_FD 42

static void mock_log(const char *fmt, ...)
{
	const char *path = getenv("WIRINGPI_MOCK_LOG");
	FILE *f = (path && *path) ? fopen(path, "a") : stdout;
	va_list ap;

	if (!f)
	{
		perror("wiringPi mock: cannot open log");
		exit(99);
	}
	va_start(ap, fmt);
	vfprintf(f, fmt, ap);
	va_end(ap);
	fputc('\n', f);
	if (f != stdout)
	{
		fclose(f);
	}
}

/* WIRINGPI_MOCK_WRITE_FAIL=1 makes all write functions fail (return -1) */
static int write_result(void)
{
	const char *fail = getenv("WIRINGPI_MOCK_WRITE_FAIL");
	return (fail && *fail == '1') ? -1 : 0;
}

int wiringPiI2CRead(int fd)                   { mock_log("read"); (void) fd; return 0; }
int wiringPiI2CReadReg8(int fd, int reg)       { mock_log("readReg8 0x%02x", reg); (void) fd; return 0; }
int wiringPiI2CReadReg16(int fd, int reg)      { mock_log("readReg16 0x%02x", reg); (void) fd; return 0; }

int wiringPiI2CWrite(int fd, int data)
{
	if (fd != MOCK_FD) mock_log("bad-fd %d", fd);
	mock_log("write 0x%02x", data & 0xFF);
	return write_result();
}

int wiringPiI2CWriteReg8(int fd, int reg, int data)
{
	if (fd != MOCK_FD) mock_log("bad-fd %d", fd);
	mock_log("writeReg8 0x%02x 0x%02x", reg & 0xFF, data & 0xFF);
	return write_result();
}

int wiringPiI2CWriteReg16(int fd, int reg, int data)
{
	if (fd != MOCK_FD) mock_log("bad-fd %d", fd);
	mock_log("writeReg16 0x%02x 0x%04x", reg & 0xFF, data & 0xFFFF);
	return write_result();
}

int wiringPiI2CSetupInterface(const char *device, int devId)
{
	mock_log("setupInterface %s 0x%02x", device, devId);
	return MOCK_FD;
}

int wiringPiI2CSetup(const int devId)
{
	const char *fail = getenv("WIRINGPI_MOCK_SETUP_FAIL");
	mock_log("setup 0x%02x", devId);
	if (fail && *fail == '1')
	{
		return -1;
	}
	return MOCK_FD;
}
