/********************************** 
*
*  video.h
*  	Galindo, Jose Ignacio
*  	Homovc, Federico
*	Reznik, Luciana
*		ITBA 2011
*
***********************************/

#ifndef _video_
#define _video_

/***	Module Defines	***/
#define START_POS  0//initial position of cursor

/***************************************************************
*k_clear_screen
*
*Clears the screen in color text mode
****************************************************************/
void k_clear_screen();


/***************************************************************
*writeScreen
*
*Writes the screen (supposed to be written in assembler)
****************************************************************/
void writeScreen(char* buffer, size_t count);


/***************************************************************
*eraseScreen
*
*Erases the screen (supposed to be written in assembler)
****************************************************************/
void eraseScreen(char* buffer, size_t count);


/***************************************************************
 * scrolldown
 *
 * Scrolls dows the video screen one line
 ***************************************************************/
void scrolldown();

/***************************************************************
 * enter
 *
 * Sets the cursor in the following line
 ***************************************************************/
void enter();

void moveCursor();

#endif

