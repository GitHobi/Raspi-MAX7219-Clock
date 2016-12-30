
#include <signal.h>
#include <poll.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <time.h>
//#include <fcntl.h>
//#include <sys/ioctl.h>
//#include <linux/spi/spidev.h>

#include "config.h"

#include <wiringPi.h>


static volatile int verbose = 0;



/*
 *
 * Configure port settings ...
 *
 */
int init_ports()
{
	if (verbose) fprintf(stdout, "setting up ports with wiringPi...\n");

    pinMode (CLOCK_LOAD, OUTPUT);
	pinMode (CLOCK_DIN , OUTPUT);
	pinMode (CLOCK_CLK, OUTPUT);

	digitalWrite(CLOCK_LOAD, 1);
	digitalWrite(CLOCK_CLK, 0);
	return 1;
}


int _writeValue(uint16_t value)
{


	//for (i = 0; i<8; i++) {
	//	max7219_PORT &= ~(1 << max7219_CLK); //Clock auf Low
	//	if (Data & (1 << 7)) {
	//		max7219_PORT |= (1 << max7219_DIN);
	//	}
	//	else{
	//		max7219_PORT &= ~(1 << max7219_DIN);
	//	}
	//	Data <<= 1;
	//	max7219_PORT |= (1 << max7219_CLK); //Clock auf High, Daten werden übernommen
	//}


	for (uint8_t i = 0; i < 16; i++)
	{
		digitalWrite(CLOCK_CLK, 0);

		if (value & (1<<15))
		{
			//printf("1");
			//printf("%i-%i\n", i, 1);
			digitalWrite(CLOCK_DIN, 1);
		}
		else
		{
			//printf("%i-%i\n", i, 0);
			//printf("0");
			digitalWrite(CLOCK_DIN, 0);
		}

		value <<= 1;

		//if (i == 8) printf(" ");
		//if (i == 0) printf(" ");

		digitalWrite(CLOCK_CLK, 1);
	}


	//for (int i = 16; i >= 0; i--)
	//{
	//	digitalWrite(CLOCK_CLK, 0);
	//	if ((value & (1 << i)) > 0)
	//	{
	//		printf("1");
	//		//printf("%i-%i\n", i, 1);
	//		digitalWrite(CLOCK_DIN, 1);
	//	}
	//	else
	//	{
	//		//printf("%i-%i\n", i, 0);
	//		printf("0");
	//		digitalWrite(CLOCK_DIN, 0);
	//	}

	//	if (i == 8) printf(" ");
	//	if (i == 0) printf(" ");

	//	
	//	digitalWrite(CLOCK_CLK, 1);
	//}
	return 1;
}

int writeValue (uint16_t value1, uint16_t value2)
{

	//digitalWrite(CLOCK_LOAD, 0);
	_writeValue(value1);
	_writeValue(value2);
	
	
	digitalWrite(CLOCK_LOAD, 1);
	//delay(1);
	digitalWrite(CLOCK_LOAD, 0);
	
	//printf("\n");

	return 1;
}






int main(int argc, char *argv[])
{

	int opt;

	int intensityInnen = 15;
	int intensityAussen = 15;

	while ((opt = getopt(argc, argv, "vi:a:")) != -1)
	{
		switch (opt)
		{
		case 'v':
			verbose = 1;
			break;
		case 'i':
			intensityInnen = atoi(optarg);
			break;
		case 'a':
			intensityAussen = atoi(optarg);
			break;
		default: /* '?' */
			fprintf(stderr, "Usage: %s [-v]\n", argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	wiringPiSetup();
	init_ports();


	
	writeValue(0x0F00, 0x0F00);  //Testmode aus
	writeValue(0x0A00+intensityInnen, 0x0A00+intensityAussen);  //Full Power!


  	return 0 ;
}
