/********************************** 
 *
 *  timertick.c
 *  	Galindo, Jose Ignacio
 *  	Homovc, Federico
 *		Reznik, Luciana
 *		ITBA 2011
 *
 ***********************************/

/***	Project Includes	***/
#include "../include/kasm.h"
#include "../include/defs.h"
#include "../include/timertick.h"

unsigned int timestick = 0;
unsigned int tickswait = 0;

void
int_08() {
	timestick++;
	tickswait++;
}

unsigned long int
getSpeedWay1() {
	unsigned long int clock_cycles;
	double seconds;
	unsigned long int ticks = timestick;
	clock_cycles = _getCPUSpeed();
	unsigned int times = 0;
	while (times < (1024 * 1024 * 100)) {
		times++;
	}
	ticks = timestick - ticks;
	clock_cycles = _getCPUSpeed() - clock_cycles;
	seconds = ticks * 0.055;
	return clock_cycles / seconds;
}

unsigned long int
getSpeedWay2(int times) {
	unsigned long int clock_cycles;
	double seconds;
	unsigned long int ticks = timestick;
	clock_cycles = _getCPUSpeed();
	while (timestick - ticks < times)
		;
	ticks = timestick - ticks;
	clock_cycles = _getCPUSpeed() - clock_cycles;
	seconds = ticks * 0.055;
	return clock_cycles / seconds;
}

unsigned long int
getCPUSpeed() {
	double ans = 0;
	ans += (double) getSpeedWay1() / (1024 * 1024);
	ans += (double) getSpeedWay1() / (1024 * 1024);
	ans += (double) getSpeedWay2(8) / (1024 * 1024);
	ans += (double) getSpeedWay2(12) / (1024 * 1024);

	return ans / 4;
}
