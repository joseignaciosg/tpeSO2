/********************************** 
 *
 *  Kernel.h
 *  	Galindo, Jose Ignacio
 *  	Homovc, Federico
 *  	Loreti, Nicolas
 *		ITBA 2011
 *
 ***********************************/

#ifndef _kernel_
#define _kernel_


/***	Project Includes	***/
#include "../include/defs.h"

/***	Module Defines	***/
#define OS_PID	0

extern KEY_BUFFER keybuffer;
extern char buffcopy[BUFFER_SIZE];

/*************************************************************************
 * Idle
 *
 * Idle process function, does nothing but staying in a loop forever.
 * **********************************************************************/
int Idle(int argc, char* argv[]);

/*************************************************************************
 * SetupScheduler
 *
 * Sets up the initial values for the scheduler and creates the Idle process.
 * **********************************************************************/
void SetupScheduler(void);

/*************************************************************************
 * awake_process
 *
 * Awakes the process with the given pid from the processes list by setting
 * its state to READY.
 * **********************************************************************/
void awake_process(int pid);

/*************************************************************************
 * sleep
 *
 * Blocks the process currently running during a given amount of time in 
 * seconds given as a parameter.
 * **********************************************************************/
void sleep(int secs);

/*************************************************************************
 * end_process
 *
 * This function is called when a process finishes. It awakes any process
 * that could be waiting for it to finish and then clears it from the 
 * processes list.
 * **********************************************************************/
void end_process(void);

/*************************************************************************
 * initializeIDT
 *
 * Initializes interrupt descriptor table.
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
void setup_IDT_entry(DESCR_INT *item, byte selector, dword offset, byte access,	byte cero);

/***********************************************************************
 * reboot
 *
 * Reboots the system
 * ********************************************************************/
void reboot();


/*************************************************************************
 * initializeIDT
 *
 * Clears the whole screen and sets the cursor position to the upper left
 * corner.
 * **********************************************************************/
void k_clear_screen();

/***********************************************************************
 * startTerminal
 *
 * Clears the terminal buffer of the given terminal and sets the key
 * buffer size to 0.
 * ********************************************************************/
void startTerminal(int pos);

/***********************************************************************
 * set_Process_ready
 *
 * Places the given process at the end of the processes list, so it can
 * be picked by the scheduler to run.
 * ********************************************************************/
void set_Process_ready(PROCESS * proc);

/***********************************************************************
 * malloc
 *
 * Returns a void pointer to an allocated buffer of the size given as a
 * parameter.
 * ********************************************************************/
void * malloc (int size);

/***********************************************************************
 * calloc
 *
 * Same as malloc but all the bytes in the buffer are set to 0.
 * ********************************************************************/
void * calloc (int size, int quant);

/***********************************************************************
 * logUser
 *
 * Asks the user to type a username and a password and compares them to the
 * ones saved in the usersfile file. If any of them matches, it creates
 * a Shell process for each terminal and sets the values of the current
 * system user to the ones of it who has loged. Otherwise, it just keeps
 * asking the user for a name and a password.
 * ********************************************************************/
void logUser(void);

/***********************************************************************
 * logout
 *
 * This function first kills all the Shell processes and then creates a 
 * logUser process so another user can be loged to the system.
 * ********************************************************************/
void logout(int argc, char * argv[]);

/***********************************************************************
 * createusr
 *
 * Creates a new user with the parameters given and writes all the 
 * related data in the usersfile file.
 * ********************************************************************/
void createusr(char * name, char * password, char * group);

void semget_in_kernel(semItem * param);

void up_in_kernel(int key);

void down_in_kernel(int key);

#endif
