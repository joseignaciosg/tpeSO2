/********************************** 
*
*  Kernel.h
*  	Galindo, Jose Ignacio
*  	Homovc, Federico
*	Reznik, Luciana
*		ITBA 2011
*
***********************************/

#ifndef _kernel_
#define _kernel_

/***	Proyect Includes	***/
#include "../include/defs.h"

/***	Module Defines	***/
#define OS_PID	0

typedef enum eUSER {U_KERNEL=0, U_NORMAL} tUSERS;
int (*player)(void);
extern KEY_BUFFER keybuffer;
extern char buffcopy[BUFFER_SIZE]; //for buffercopy()
//typedef enum eINT_80 {WRITE=0, READ=1} tINT_80;
//extern char copy[BUFFER_SIZE]; //for strcopy()


/*****************************
 * initializeIDT
 *
 * Intializes the key buffer
 * ****************************/
void initializeIDT();


/*****************************
 * unmaskPICS
 *
 * Clears the IMR of both master and slave pics
 * ****************************/
void unmaskPICS();

/***************************************************************
*setup_IDT_entry
* Inicializa un descriptor de la IDT
*
*Recibe: Puntero a elemento de la IDT
*	 Selector a cargar en el descriptor de interrupcion
*	 Puntero a rutina de atencion de interrupcion
*	 Derechos de acceso del segmento
*	 Cero
****************************************************************/
void setup_IDT_entry (DESCR_INT *item, byte selector, dword offset, byte access, byte cero);

/*****************************
 * reboot
 *
 * Reboots the system
 * ****************************/
void reboot();

#endif
