/********************************** 
*
*  apps.h
*  	Galindo, Jose Ignacio
*  	Homovc, Federico
*  	Loreti, Nicolas
*		ITBA 2011
*
***********************************/

#ifndef _apps_
#define _apps_

extern int last100[100];
extern int currentProcessTTY;

/***************************************************************************
 * prioridad
 *
 * This function does nothing but staying for ever in a loop.
 ***************************************************************************/
void prioridad(int argc, char * argv[]);

/***************************************************************************
 * prueba
 *
 * This function does nothing but printing "prueba" and staying for ever 
 * in a loop.
 ***************************************************************************/
void prueba(int argc, char * argv[]);

/***************************************************************************
 * prueba2
 *
 * This function prints "prueba2", then creates three "prueba" processes, 
 * and finally counts to 50000000 and ends.
 ***************************************************************************/
void prueba2(int argc, char * argv[]);

/***************************************************************************
 * top
 *
 * This function shows all the processes in the system that have runned
 * recently. Prints their name, PID and how much of the processor each is 
 * occupying, and refreshes these values every three seconds.
 ***************************************************************************/
void top(int argc, char * argv[]);

void fifo_writer_test(int argc, char * argv[]);

void fifo_reader_test(int argc, char * argv[]);


#endif
