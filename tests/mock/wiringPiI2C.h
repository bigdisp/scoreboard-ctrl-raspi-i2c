/*
 * Host-side mock of wiringPi's I2C API (wiringPiI2C.h).
 *
 * Used only by the PC test suite in tests/. Instead of talking to /dev/i2c-*,
 * every call is appended as one line to the file named by the environment
 * variable WIRINGPI_MOCK_LOG (default: stdout), e.g.
 *
 *   setup 0x10
 *   write 0x41
 *   writeReg16 0x00 0x1234
 *
 * Set WIRINGPI_MOCK_SETUP_FAIL=1 to make wiringPiI2CSetup() return -1,
 * WIRINGPI_MOCK_WRITE_FAIL=1 to make all write functions return -1.
 */
#ifndef WIRINGPI_I2C_MOCK_H
#define WIRINGPI_I2C_MOCK_H

#ifdef __cplusplus
extern "C" {
#endif

int wiringPiI2CRead(int fd);
int wiringPiI2CReadReg8(int fd, int reg);
int wiringPiI2CReadReg16(int fd, int reg);

int wiringPiI2CWrite(int fd, int data);
int wiringPiI2CWriteReg8(int fd, int reg, int data);
int wiringPiI2CWriteReg16(int fd, int reg, int data);

int wiringPiI2CSetupInterface(const char *device, int devId);
int wiringPiI2CSetup(const int devId);

#ifdef __cplusplus
}
#endif

#endif
