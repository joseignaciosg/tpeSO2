/********************************** 
*
*  timertick.h
*  	Galindo, Jose Ignacio
*  	Homovc, Federico
*	Reznik, Luciana
*		ITBA 2011
*
***********************************/

#ifndef _timertick_
#define _timertick_


/*
 * int_08
 *
 * Each time a interrupt from IRO0 (timertick) comes across,
 * this function is excecuted
 *
 */
void int_08();

unsigned long int getCPUSpeed();

#endif
