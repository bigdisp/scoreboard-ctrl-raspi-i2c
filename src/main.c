#include "wiringPiI2C.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * List of statuscodes
 */
typedef enum e_status__
{
	E_CONNECTION_ERROR = -2,
	E_NOT_ENOUGH_ARGS = -1,
	E_SUCCESS = 0,
	E_FAILURE = 1,
	E_FAILURE_PWM,
	E_FAILURE_SEND_CMD,
	E_FAILURE_ILLEGAL_FD,
} e_status;

/**
 * Run the pwm functionalities.
 *
 * @param fd file descriptor for i2c connection
 * @param argc number of arguments
 * @param argv the arguments given
 * @return e_status
 */
e_status pwm(int fd, int argc, char * argv[])
{
	if (!fd)
	{
		return E_FAILURE_ILLEGAL_FD;
	}

	if(argc < 4)
	{
		//Interpretiere als "Stufe" 0 - 10
		//fprintf(stdout, "Writing empty output\n");
		int high,low,max,min;
		max = 10000;
		min = 200;
		high = ((max - min) / 10 * atoi(argv[2])) + min;
		low  = ((max - min) / 10 * (10 - atoi(argv[2]))) + min;
		wiringPiI2CWriteReg16(fd, 0x00, high);
		wiringPiI2CWriteReg16(fd, 0x01, low);
		wiringPiI2CWrite(fd, 0x04);
	}
	else
	{
		//Interpretiere als direkt zu schreibende Werte:
		//fprintf(stdout, "Writing: %c\n", *argv[2]);
		wiringPiI2CWriteReg16(fd, 0x00, atoi(argv[2]));
		wiringPiI2CWriteReg16(fd, 0x01, atoi(argv[3]));
		wiringPiI2CWrite(fd, 0x04);
	}
	return E_SUCCESS;
}

/**
 * Enable PWM.
 *
 * @param fd file descriptor for i2c connection
 * @return E_SUCCESS
 */
e_status pwm_enable(int fd)
{
	wiringPiI2CWrite(fd, 0x03);
	return E_SUCCESS;
}

/**
 * Disable PWM.
 *
 * @param fd file descriptor for i2c connection
 * @return E_SUCCESS
 */
e_status pwm_disable(int fd)
{
	wiringPiI2CWrite(fd, 0x02);
	return E_SUCCESS;
}

/**
 * Send letter or digit to display.
 *
 * @param fd file descriptor for i2c connection
 * @param argc number of arguments
 * @param argv the arguments given
 * @return E_NOT_ENOUGH_ARGS
 */
e_status send_cmd(int fd, int argc, char * argv[])
{
	if(argc < 3)
	{
		//fprintf(stdout, "Writing empty output\n");
		wiringPiI2CWrite(fd, ' ');
	}
	else
	{
		//fprintf(stdout, "Writing: %c\n", *argv[2]);
		wiringPiI2CWrite(fd, *argv[2]);
		//we can write 2 databytes with wiringPiI2CWriteReg8() and 3 with wiringPiI2CWriteReg16(), including the address
	}
	return E_SUCCESS;
}

/**
 * Generate error message for too few arguments on stderr.
 *
 * @param argc number of arguments
 * @param argv the arguments given
 * @return E_NOT_ENOUGH_ARGS
 */
e_status not_enough_args(int argc, char * argv[])
{
	fprintf(stderr, "%s: ", argv[0]);
	fprintf(stderr, "Not enough arguments\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "This program provides low level access to the scoreboard display.\n");
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "\t%s target [value]\n", argv[0]);
	fprintf(stderr, "\t%s pwm value|on|off [value]\n", argv[0]);
	fprintf(stderr, "\t\ttarget: The Target number to be changed.\n");
	fprintf(stderr, "\t\t        Abbreviations from list below.\n");
	fprintf(stderr, "\t\tvalue:  The ASCII Symbol to write. Can be any ASCII value,\n");
	fprintf(stderr, "\t\t        but unknown values are replaced by a '-'.\n");
	fprintf(stderr, "\t\t        If string is provided, only the first symbol is written.\n");
	fprintf(stderr, "\t\tpwm:    Change the PWM settings.\n");
	fprintf(stderr, "\t\t        If a single value is given, it is interpreted as brightness\n");
	fprintf(stderr, "\t\t        steps between 0 and 10. 10 is brightest\n");
	fprintf(stderr, "\t\t        If two values are given, they are interpreted as clock cycles.\n");
	fprintf(stderr, "\t\t        The first value specifies the on-time, the second the off-time.\n");
	fprintf(stderr, "\t\t        If 'off' is given instead of the first value, pwm is disabled.\n");
	fprintf(stderr, "\t\t        In that case, the led are enabled permanently.\n");
	fprintf(stderr, "\t\t        Replacing the value by 'on' reenables pwm.\n");
	fprintf(stderr, "\t\t        This is not done automatically if values are changed.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "\t\tAbbreviation list:\n");
	fprintf(stderr, "\t\t\tH1:  Home ones\n");
	fprintf(stderr, "\t\t\tH10: Home tens\n");
	fprintf(stderr, "\t\t\tR1:  Road ones\n");
	fprintf(stderr, "\t\t\tR10: Road tens\n");
	fprintf(stderr, "\t\t\tI1:  Inning ones\n");
	fprintf(stderr, "\t\t\tI10: Inning tens\n");

	return E_NOT_ENOUGH_ARGS;
}

/**
 * Main function to be executed
 */
int main(int argc, char * argv[])
{
	// Check the arguments
	if(argc < 2)
	{
		return not_enough_args(argc, argv);
	}
	int fd;
	//TODO: Get the device ID from a list to connect to
	fd = wiringPiI2CSetup(0x10);
	if(-1 == fd)
	{
		fprintf(stderr, "%s: ", argv[0]);
		fprintf(stderr, "Error while connecting to i2c device\n");
		return E_CONNECTION_ERROR;
	}
	
	if(strcmp(argv[1], "pwm") == 0)
	{
		if(argc < 3)
		{
			return not_enough_args(argc, argv);
		}

		if(strcmp(argv[2], "on"))
		{
			return pwm_enable(fd);
		}
		else if(strcmp(argv[2], "off"))
		{
			return pwm_disable(fd);
		}
		return pwm(fd, argc, argv);
	}

	return send_cmd(fd, argc, argv);
}

//Es gibt:
/*
wiringPiI2CRead(int fd);
wiringPiI2CWrite(int fd, int data);
wiringPiI2CReadReg8(int fd, int reg);
wiringPiI2CReadReg16(int fd, int reg);
wiringPiI2CWriteReg8(int fd, int reg, int data);
wiringPiI2CWriteReg16(int fd, int reg, int data;
*/
