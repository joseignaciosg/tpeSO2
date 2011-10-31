/********************************** 
 *
 *  timertick.h
 *  	Galindo, Jose Ignacio
 *  	Homovc, Federico
 *		ITBA 2011
 *
 ***********************************/

#ifndef _timertick_
#define _timertick_

/***********************************************************************
 * int_08
 *
 * Each time a interrupt from IRO0 (timertick) comes across,
 * this function is executed
 *
 **********************************************************************/
void int_08();

/*********************************************************************
 *getSpeedWay1
 *
 *Returns the CPU's speed not using the timer tick in the
 *main cycle
 *Is called from getCPUSpeed
 *********************************************************************/
unsigned long int getSpeedWay1();

/*********************************************************************
 *getSpeedWay2
 *
 *Returns the CPU's speed using the timer tick in the
 *main cycle
 *Is called from getCPUSpeed
 *********************************************************************/
unsigned long int getSpeedWay2(int times);

/********************************************************************
 *getCPUSpeed
 *
 *Returns the CPU's speed
 *********************************************************************/
unsigned long int getCPUSpeed();

#endif
