#include "wiringPiI2C.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * i2c opcodes of the scoreboard_atmega8_i2c firmware protocol this tool was
 * written for (firmware state of 2016-04, single pwm channel).
 * Note: later firmware versions use a different opcode layout.
 */
#define CMD_PWM_ON_TIME  0x00
#define CMD_PWM_OFF_TIME 0x01
#define CMD_PWM_DISABLE  0x02
#define CMD_PWM_ENABLE   0x03
#define CMD_PWM_APPLY    0x04

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
	if (fd < 0)
	{
		return E_FAILURE_ILLEGAL_FD;
	}

	int high, low;
	if(argc < 4)
	{
		//Interpretiere als "Stufe" 0 - 10
		const int max = 10000;
		const int min = 200;
		int step = atoi(argv[2]);
		if (step < 0)
		{
			step = 0;
		}
		else if (step > 10)
		{
			step = 10;
		}
		high = ((max - min) / 10 * step) + min;
		low  = ((max - min) / 10 * (10 - step)) + min;
	}
	else
	{
		//Interpretiere als direkt zu schreibende Werte:
		high = atoi(argv[2]);
		low  = atoi(argv[3]);
	}

	// Store on/off time and apply the new timing.
	// Enabling/disabling pwm is left to the 'on'/'off' commands.
	if (wiringPiI2CWriteReg16(fd, CMD_PWM_ON_TIME, high & 0xFFFF) < 0
		|| wiringPiI2CWriteReg16(fd, CMD_PWM_OFF_TIME, low & 0xFFFF) < 0
		|| wiringPiI2CWrite(fd, CMD_PWM_APPLY) < 0)
	{
		return E_FAILURE_PWM;
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
	if (wiringPiI2CWrite(fd, CMD_PWM_ENABLE) < 0)
	{
		return E_FAILURE_PWM;
	}
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
	if (wiringPiI2CWrite(fd, CMD_PWM_DISABLE) < 0)
	{
		return E_FAILURE_PWM;
	}
	return E_SUCCESS;
}

/**
 * Send letter or digit to display.
 *
 * @param fd file descriptor for i2c connection
 * @param argc number of arguments
 * @param argv the arguments given
 * @return E_SUCCESS or E_FAILURE_SEND_CMD
 */
e_status send_cmd(int fd, int argc, char * argv[])
{
	int symbol = ' ';
	if(argc >= 3 && argv[2][0] != '\0')
	{
		symbol = (unsigned char) argv[2][0];
	}
	//we can write 2 databytes with wiringPiI2CWriteReg8() and 3 with wiringPiI2CWriteReg16(), including the address
	if (wiringPiI2CWrite(fd, symbol) < 0)
	{
		return E_FAILURE_SEND_CMD;
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
	(void) argc;
	fprintf(stderr, "%s: ", argv[0]);
	fprintf(stderr, "Not enough arguments\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "This program provides low level access to the scoreboard display.\n");
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "\t%s target [digit]\n", argv[0]);
	fprintf(stderr, "\t%s pwm value|on|off [value]\n", argv[0]);
	fprintf(stderr, "\t\ttarget: The Target number to be changed.\n");
	fprintf(stderr, "\t\t        Abbreviations from list below.\n");
	fprintf(stderr, "\t\tdigit:  The ASCII Symbol to write. Can be any ASCII value,\n");
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

		if(strcmp(argv[2], "on") == 0)
		{
			return pwm_enable(fd);
		}
		else if(strcmp(argv[2], "off") == 0)
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
