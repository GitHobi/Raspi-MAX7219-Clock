

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
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
//#include <fcntl.h>
//#include <sys/ioctl.h>
//#include <linux/spi/spidev.h>

#include <inttypes.h>
#include <math.h>
#include <stdio.h>



#include "config.h"

#include <wiringPi.h>


static volatile int keepRunning = 1;
static volatile int verbose = 0;
static volatile int foreground = 0;






uint8_t minutes[8];
uint8_t hours[8];


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
	uint8_t i = 0;
	for (i = 0; i < 16; i++)
	{
		digitalWrite(CLOCK_CLK, 0);

		if (value & (1<<15))
		{
			digitalWrite(CLOCK_DIN, 1);
		}
		else
		{
			digitalWrite(CLOCK_DIN, 0);
		}

		value <<= 1;

		digitalWrite(CLOCK_CLK, 1);
	}

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




void setHour(uint8_t hour, uint8_t inner, uint8_t value)
{
	
	if (hour != 0)
	{
		if (hour > 12) hour = hour - 12;
		hour = 12 - hour;
	}

	//                          00 01 02 03 04 05 06 07  08 09 10 11  12 13 14 15 16 17 18 19 20 21 22 23   
	uint8_t slotMapping1[24] = { 0, 0, 0, 0, 0, 0, 0, 0,  4, 4, 4, 4,  1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 4 };
	uint8_t ledMapping1[24] =  { 3, 7, 2, 4, 0, 5, 1, 6,  3, 7, 2, 4,  0, 5, 1, 6, 3, 7, 2, 4, 0, 5, 1, 6 };

	uint8_t slotMapping2[24] = { 4, 4, 4, 4, 6, 6, 6, 6,  6, 6, 6, 6,  1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 4 };
	uint8_t ledMapping2[24] =  { 0, 5, 1, 6, 3, 7, 2, 4,  0, 5, 1, 6,  4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7 };


	uint8_t slot = 0;
	uint8_t led = 0;

	if (inner == 0)
	{
		slot = slotMapping1[hour];
		led = ledMapping1[hour];
	}
	else
	{
		slot = slotMapping2[hour];
		led = ledMapping2[hour];
	}

	//printf("%2i -> %2i %2i\n", hour, slot, led);


	if (value > 0)
	{
		hours[slot] |= (1 << led);
	}
	else
	{
		hours[slot] &= ~(1 << led);
	}


	


}



int setMinute(uint8_t min, uint8_t value)
{

	min = 60 - min;

	if (min == 0) min = 59;
	else min = min - 1;
	

	uint8_t slot = min / 8 +1;
	uint8_t led = min % 8 +1;

	//printf("%2i -> %2i, %2i\n", min, slot, led);

	
	switch (led)
	{
		// LED
		// 7 = 1
		// 5 = 0
		// 3 = 2
		// 1 = 3 ?
		// 4 = 4
		// 6 = 5
		// 8 = 6
		// 2 = 7
	case 5: led = 0; break;
	case 7: led = 1; break;
	case 3: led = 2; break;
	case 1: led = 3; break;
	case 4: led = 4; break;
	case 6: led = 5; break;
	case 8: led = 6; break;
	case 2: led = 7; break;
	}

	

	switch (slot)
	{
		// 5 = 0
		// 1 = 1
		// 8 = 2
		// 4 = 3
		// 6 = 4
		// 2 = 5
		// 7 = 6
		// 3 = 7
	case 5: slot = 0; break;
	case 1: slot = 1; break;
	case 8: slot = 2; break;
	case 4: slot = 3; break;
	case 6: slot = 4; break;
	case 2: slot = 5; break;
	case 7: slot = 6; break;
	case 3: slot = 7; break;
	}


	int retValue = (minutes[slot] & (1 << led)) > 0 ? 1 : 0;
		
	if (value==1)
	{
		
		minutes[slot] = minutes[slot] | (1 << led);
	}
	if ( value == 0)
	{
		minutes[slot] = minutes[slot] & ~(1 << led);
	}

	if (value == 2)
	{
		minutes[slot] = minutes[slot] ^(1 << led);
	}


	return retValue;
	//printf("%2i -> %2i, %2i\n", min, slot, led);
}

void transferBuffer()
{
	writeValue(0x0100 + hours[0], 0x0100 + minutes[0]);
	writeValue(0x0200 + hours[1], 0x0200 + minutes[1]);
	writeValue(0x0300 + hours[2], 0x0300 + minutes[2]);
	writeValue(0x0400 + hours[3], 0x0400 + minutes[3]);
	writeValue(0x0500 + hours[4], 0x0500 + minutes[4]);
	writeValue(0x0600 + hours[5], 0x0600 + minutes[5]);
	writeValue(0x0700 + hours[6], 0x0700 + minutes[6]);
	writeValue(0x0800 + hours[7], 0x0800 + minutes[7]);
}

void clearMinutes()
{
	uint8_t i = 0;
	for (i = 0; i < 8; i++)
	{
		minutes[i] = 0;
	}
}

void clearAll()
{
	uint8_t i = 0;
	for (i = 0; i < 8; i++)
	{
		minutes[i] = 0;
		hours[i] = 0;
	}

	
	setHour(0, 1, 1);
	setHour(1, 1, 1);
	setHour(2, 1, 1);
	setHour(3, 1, 1);
	setHour(4, 1, 1);
	setHour(5, 1, 1);
	setHour(6, 1, 1);
	setHour(7, 1, 1);
	setHour(8, 1, 1);
	setHour(9, 1, 1);
	setHour(10, 1, 1);
	setHour(11, 1, 1);

	

	transferBuffer();

}

//
//void displaytest_on(void)
//{
//	digitalWrite(CLOCK_LOAD, 0);
//	_writeValue(0x0F01);
//	_writeValue(0x0F01);
//	digitalWrite(CLOCK_LOAD, 1);
//
//}
//void displaytest_off(void)
//{
//	digitalWrite(CLOCK_LOAD, 0);
//	_writeValue(0x0F00);
//	_writeValue(0x0F00);
//	digitalWrite(CLOCK_LOAD, 1);
//
//}
//

void intHandler(int dummy)
{
	syslog(LOG_NOTICE, "Received command to terminate!");
	keepRunning = 0;
}


static void skeleton_daemon()
{
	pid_t pid;

	/* Fork off the parent process */
	pid = fork();

	/* An error occurred */
	if (pid < 0)
		exit(EXIT_FAILURE);

	/* Success: Let the parent terminate */
	if (pid > 0)
		exit(EXIT_SUCCESS);

	/* On success: The child process becomes session leader */
	if (setsid() < 0)
		exit(EXIT_FAILURE);

	/* Catch, ignore and handle signals */
	//TODO: Implement a working signal handler */
	signal(SIGCHLD, intHandler);
	signal(SIGHUP, intHandler);

	/* Fork off for the second time*/
	pid = fork();

	/* An error occurred */
	if (pid < 0)
		exit(EXIT_FAILURE);

	/* Success: Let the parent terminate */
	if (pid > 0)
		exit(EXIT_SUCCESS);

	/* Set new file permissions */
	umask(0);

	/* Change the working directory to the root directory */
	/* or another appropriated directory */
	chdir("/");

	/* Close all open file descriptors */
	int x;
	for (x = sysconf(_SC_OPEN_MAX); x >= 0; x--)
	{
		close(x);
	}
}



int main(int argc, char *argv[])
{

	int opt;

	while ((opt = getopt(argc, argv, "fv")) != -1)
	{
		switch (opt)
		{
		case 'v':
			verbose = 1;
			break;
		case 'f':
			foreground = 1;
			break;
		default: /* '?' */
			fprintf(stderr, "Usage: %s [-vf]\n", argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	if (foreground == 0)
	{
		printf("daemonzing ...\n");
		skeleton_daemon();
	}

	/* Open the log file */
	openlog("clock", LOG_PID, LOG_DAEMON);
	syslog(LOG_NOTICE, "Process started!");


	wiringPiSetup();
	init_ports();


	//signal(SIGHUP, intHandler);
	signal(SIGINT, intHandler);
	signal(SIGKILL, intHandler);


	clearAll();
	

	writeValue(0x0F00, 0x0F00);  //Testmode aus
	writeValue(0x0900, 0x0900);  //No Decode
	writeValue(0x0A00, 0x0A05);  //Full Power!
	writeValue(0x0B07, 0x0B07);  //Alle Digits
	writeValue(0x0C01, 0x0C01);  //Normal mode


	

	int omin = -1;
	int i = 0;
	for (i = 0; i < 60; i++)
	{
		if (omin > -1) setMinute(omin, 0);
		setMinute(i, 1);
		omin = i;
		setHour(i / 5, 0, 1);
		transferBuffer();
		delay(10);
	}
	clearAll();


	//uint8_t min = 0;

	uint8_t oldHour = -1;
	uint8_t oldMinute = -1;
	uint8_t oldSec = -1;

	//long int o = 0;
	while (keepRunning)
    {

		time_t epoch_time;
		struct tm *tm_p;
		epoch_time = time(NULL);
		tm_p = localtime(&epoch_time);

		uint8_t min = tm_p->tm_min;
		uint8_t hour = tm_p->tm_hour;
		uint8_t sec = tm_p->tm_sec;

		
		if ((oldMinute != min) || (oldHour != hour) || (oldSec != sec))
		{

			//clearAll();
			if (oldMinute >= 0) setMinute(oldMinute, 0);
			if (oldHour >= 0) setHour(oldHour, 0, 0);
			if (oldSec >= 0) setMinute(oldSec, 0);

			setMinute(min, 1);
			setMinute(sec, 1);
			setHour(hour, 0, 1);
			oldMinute = min;
			oldHour = hour;
			oldSec = sec;

			transferBuffer();
			

			{
				/*
				*  alle Minuten Leds von der aktuellen Minute an im Kreis innerhalb einer Sekunde aufleuchten lassen ...
				*/
				//epoch_time = time(NULL);
				//tm_p = localtime(&epoch_time);


				struct timeval tv;
				struct timezone tz;

				//gettimeofday(&tv, &tz);
				//long int start_time;
				//long int time_difference;
				//struct timespec gettime_now;

				//clock_gettime(CLOCK_REALTIME, &gettime_now);
				//start_time = gettime_now.tv_nsec;		//Get nS value

				

				//printf("%li\n", (tv.tv_sec*1000000+tv.tv_usec)-o);
				//printf("\n");

				//o = tv.tv_sec * 1000000 + tv.tv_usec;

				int usectotal = 950*1000;
				//int a = 0;
				int om = -1;
				int ov = 0;

				gettimeofday(&tv, &tz);
				for (int i = 0; i < 61; i++)
				{
					if (om>-1) setMinute(om, ov);

					uint8_t p = sec + i;
					p = p % 60;
					ov = setMinute(p, 2);
					om = p;

					transferBuffer();


						struct timeval tv2;
						struct timezone tz2;
						gettimeofday(&tv2, &tz2);

						long int delta = ((tv2.tv_sec - tv.tv_sec) * 1000000 + (tv2.tv_usec - tv.tv_usec));
						tv = tv2;

						usectotal = usectotal - delta;
						//printf("%li us - remaining: %i\n", delta, usectotal);

						int d = (usectotal / 1000) / (61 - i);


							//printf("%li us - remaining: %i, %i\n", delta, usectotal, i);


						if ( (d > 0) && (d<2000)) delay(d);

				}
				setMinute(om, ov);
				

				//clearMinutes();
				
			}

			
		}
		delay(10);

		

		//min++;
		//min = min % 60;

    }


  	return 1 ;
}
