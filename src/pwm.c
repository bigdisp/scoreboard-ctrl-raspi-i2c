#include <wiringPiI2C.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char * argv[])
{
	// Check the arguments
	if(argc < 2)
	{
		fprintf(stderr, "Not enough arguments\n");
		return -1;
	}
	int fd;
	//TODO: Get the device ID from a list to connect to
	fd = wiringPiI2CSetup(0x10);
	if(-1 == fd)
	{
		fprintf(stderr, "%s: ", argv[0]);
		fprintf(stderr, "Error while connecting to i2c device\n");
		return -2;
	}
	
	if(argc < 3)
	{
		//Interpretiere als "Stufe" 0 - 10
		//fprintf(stdout, "Writing empty output\n");
		int high,low,max,min;
		max = 10000;
		min = 200;
		high = ((max - min) / 10 * atoi(argv[1])) + min;
		low  = ((max - min) / 10 * (10 - atoi(argv[1]))) + min;
		wiringPiI2CWriteReg16(fd, 0x00, high);
		wiringPiI2CWriteReg16(fd, 0x01, low);
		wiringPiI2CWrite(fd, 0x04);
	}
	else
	{
		//Interpretiere als direkt zu schreibende Werte:
		//fprintf(stdout, "Writing: %c\n", *argv[2]);
		wiringPiI2CWriteReg16(fd, 0x00, atoi(argv[1]));
		wiringPiI2CWriteReg16(fd, 0x01, atoi(argv[2]));
		wiringPiI2CWrite(fd, 0x04);
	}	
	return 0;
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
