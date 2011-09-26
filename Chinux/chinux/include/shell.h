/********************************** 
*
*  shell.h
*  	Galindo, Jose Ignacio
*  	Homovc, Federico
*	Reznik, Luciana
*		ITBA 2011
*
***********************************/

#ifndef _shell_
#define _shell_

/***	Module Defines	***/
#define SHELL_LINE  "Chinux$ "
#define SHELL_LINE_LENGTH 8

/*
* global variables used for the keyboard
*/
extern KEY_BUFFER keybuffer;
extern int keypressed;
extern int enterpressed;
extern unsigned shift_state;
extern unsigned ctrl_state;
extern unsigned alt_state;
extern unsigned supr_state;
extern unsigned up_arrow_state ;
extern unsigned down_arrow_state ;
extern char buffcopy[BUFFER_SIZE];

/*
 * global vaiables used for the timertick
 */
extern unsigned int timestick;
extern unsigned int tickswait;

/* 
 * for scanf tests 
 */
enum scan_test {NOTSCAN=0, SCANSTRING=1, SCANINT=2, SCANDOUBLE=3};


/********************************************
 * printShellLine
 *
 * prints on screen the shell line beginning
 ********************************************/
void printShellLine();

/*********************************************
 * showHelp
 *
 * Displays available commands of the shell
 ********************************************/
void showHelp();


/********************************************
 * parseBuffer
 *
 *Parses the actual keyboard buffer and calls the
 *correct function to deal with it
 ********************************************/
void parseBuffer();


/*******************************************
 * wait
 *
 * Puts the shell to sleep for sec seconds
 ********************************************/
void wait(double sec);


/*******************************************
 * shell
 *
 * Initiates the command shell
 ********************************************/
void shell(int argc, char* argv[]);

#endif
