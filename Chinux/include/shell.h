/************************************************************************
 *
 *  shell.h
 *  	Galindo, Jose Ignacio
 *  	Homovc, Federico
 *		ITBA 2011
 *
 ************************************************************************/

#ifndef _shell_
#define _shell_

/***	Module Defines	***/
#define SHELL_LINE  "Chinux$ "

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
extern unsigned up_arrow_state;
extern unsigned down_arrow_state;
extern char buffcopy[BUFFER_SIZE];

/*
 * global variables used for the video
 */
extern char * splash_screen[25];

/*
 * global variables used for the timertick
 */
extern unsigned int timestick;
extern unsigned int tickswait;

/*********************************************************************
 * printShellLine
 *
 * Prints on screen the shell line beginning
 *********************************************************************/
void printShellLine();

/*********************************************************************
 * showHelp
 *
 * Displays available commands of the shell
 ********************************************************************/
void showHelp();

/*********************************************************************
 * showSplashScreen
 *
 * Prints in screen the splash screen
 *********************************************************************/
void
showSplashScreen();

/*********************************************************************
 * wait
 *
 * Puts the shell to sleep for sec seconds
 *********************************************************************/
void wait(int sec);

/*********************************************************************
 * showLastCommand
 *
 * Prints in screen the last command
 * Used when an up arrow is pressed in
 *********************************************************************/
void showLastCommand();

/*********************************************************************
 * showPreviousCommand
 *
 * Prints in screen the previous command
 * Used when an up arrow is pressed in
 *********************************************************************/
void showPreviousCommand();

/*********************************************************************
 * saveCommand
 *
 * Saves a command in storedComm so it can be called from
 * showPreviousComand or from show lastCommand
 *********************************************************************/
void saveCommand();

/*********************************************************************
 * parseBuffer
 *
 *Parses the actual keyboard buffer and calls the
 *correct function to deal with it
 ********************************************************************/
int parseBuffer();

/*********************************************************************
 * shell
 *
 * An infinite loop that shows the shell, prints a key if pressed,and
 * when the enter is pressed it parses the line or proceed to the next
 * line.
 * If ctrl+alt+supr is pressed it reboots the system.
 *********************************************************************/
void shell();

#endif
