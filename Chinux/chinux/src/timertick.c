/********************************** 
*
*  timertick.c
*  	Galindo, Jose Ignacio
*  	Homovc, Federico
*	Reznik, Luciana
*		ITBA 2011
*
***********************************/

/***	Proyect Includes	***/
#include "../include/kasm.h"
#include "../include/defs.h"
#include "../include/timertick.h"
//#include "../include/utils.h"
//#include "../include/kernel.h"
//#include "../include/shell.h"
//#include "../include/video.h"
//#include "../include/kc.h"
//#include "../include/stdio.h"


unsigned int timestick = 0;
unsigned int tickswait = 0;

void int_08()
{
     timestick++;
     tickswait++;
}


unsigned long int
getCPUSpeed(){
	unsigned long int clock_cycles;
	double seconds;
	unsigned long int ticks=0;
	unsigned long int ans;
	int simtimes =0;
	for ( ;simtimes < 20 ; simtimes++ ){
		timestick = 0;
		clock_cycles = _getCPUSpeed(); /*returns quantity of clock cycles */
		wait(0.1);
		ticks = timestick;
		clock_cycles = _getCPUSpeed() - clock_cycles;
		seconds = ticks*0.055;
		ans += clock_cycles / seconds;
	}
	return ans/simtimes;
}
