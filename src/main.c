#include "wiringPiI2C.h"
#include <stdio.h>

int main(int argc, char * argv[])
{
	// Check the arguments
	if(argc < 2)
	{
		fprintf(stderr, "%s: ", argv[0]);
		fprintf(stderr, "Not enough arguments\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "This program provides low level access to the scoreboard display.\n");
		fprintf(stderr, "Usage:\n");
		fprintf(stderr, "\t%s target [value]\n", argv[0]);
		fprintf(stderr, "\t\ttarget: The Target number to be changed.\n");
		fprintf(stderr, "\t\t        Abbreviations from list below.\n");
		fprintf(stderr, "\t\tvalue:  The ASCII Symbol to write. Can be any ASCII value,\n");
		fprintf(stderr, "\t\t        but unknown values are replaced by a '-'.\n");
		fprintf(stderr, "\t\t        If string is provided, only the first symbol is written.\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "\t\tAbbreviation list:\n");
		fprintf(stderr, "\t\t\tH1:  Home ones\n");
		fprintf(stderr, "\t\t\tH10: Home tens\n");
		fprintf(stderr, "\t\t\tR1:  Road ones\n");
		fprintf(stderr, "\t\t\tR10: Road tens\n");
		fprintf(stderr, "\t\t\tI1:  Inning ones\n");
		fprintf(stderr, "\t\t\tI10: Inning tens\n");

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
		//fprintf(stdout, "Writing empty output\n");
		wiringPiI2CWrite(fd, ' ');
	}
	else
	{
		//fprintf(stdout, "Writing: %c\n", *argv[2]);
		wiringPiI2CWrite(fd, *argv[2]);
		//we can write 2 databytes with wiringPiI2CWriteReg8() and 3 with wiringPiI2CWriteReg16(), including the address
	}	
	return 0;
}
