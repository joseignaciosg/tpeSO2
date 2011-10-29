/********************************** 
 *
 *  Kernel.h
 *  	Galindo, Jose Ignacio
 *  	Homovc, Federico
 *		ITBA 2011
 *
 ***********************************/

#ifndef _kernel_
#define _kernel_

/***	Project Includes	***/
#include "../include/defs.h"

/***	Module Defines	***/
#define OS_PID	0

typedef enum eUSER {
	U_KERNEL = 0, U_NORMAL
} tUSERS;
int (*player)(void);
extern KEY_BUFFER keybuffer;
extern char buffcopy[BUFFER_SIZE];


/*************************************************************************
 * initializeIDT
 *
 * Initializes the key buffer
 * **********************************************************************/
void initializeIDT();

/************************************************************************
 * unmaskPICS
 *
 * Clears the IMR of both master and slave pics
 * *********************************************************************/
void unmaskPICS();

/************************************************************************
 *setup_IDT_entry
 * Initialize an IDT descriptor
 *
 *Receives: Pointer to an IDT element
 *
 *	 Selector to load in the interrupt routine
 *	 Pointer to the interrupt attention routine
 *	 Segments access rights
 *	 Zero
 ***********************************************************************/
void setup_IDT_entry(DESCR_INT *item, byte selector, dword offset, byte access,
		byte cero);

/***********************************************************************
 * reboot
 *
 * Reboots the system
 * ********************************************************************/
void reboot();


void startTerminal(int pos);

void set_Process_ready(PROCESS * proc);

void * malloc (int size);

void * calloc (int size, int quant);

void logUser(void);

void logout(int argc, char * argv[]);


#endif
