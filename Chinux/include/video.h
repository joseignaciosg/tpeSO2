/********************************** 
 *
 *  video.h
 *  	Galindo, Jose Ignacio
 *  	Homovc, Federico
 *		Reznik, Luciana
 *		ITBA 2011
 *
 ***********************************/

#ifndef _video_
#define _video_

/***************************************************************
 *k_clear_screen
 *
 *Clears the screen in color text mode
 ****************************************************************/
void k_clear_screen();

/***************************************************************
 *writeScreen
 *
 *Writes the screen
 ****************************************************************/
void writeScreen(char* buffer, size_t count);

/***************************************************************
 *eraseScreen
 *
 *Erases the screen
 ****************************************************************/
void eraseScreen(char* buffer, size_t count);

/***************************************************************
 * scrolldown
 *
 * Scrolls down the video screen one line
 ***************************************************************/
void scrolldown();

/***************************************************************
 * enter
 *
 * Sets the cursor in the following line
 ***************************************************************/
void enter();

/****************************************************************
 * moveCursor
 *
 *  Moves the cursor position
 ****************************************************************/
void moveCursor();

#endif

