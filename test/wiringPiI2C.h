/**
 * Test double for the wiringPi i2c API.
 * Only used by the unit tests in this directory, the real header is
 * provided by the wiringPi library.
 */
#ifndef WIRINGPI_I2C_STUB_H
#define WIRINGPI_I2C_STUB_H

int wiringPiI2CSetup(const int devId);
int wiringPiI2CWrite(int fd, int data);
int wiringPiI2CWriteReg16(int fd, int reg, int data);

#endif
