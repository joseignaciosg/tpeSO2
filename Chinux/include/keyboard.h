/*************************************************************************
 *  utils.h
 *  Keyboard Handling Functions
 *  	Galindo, Jose Ignacio
 *  	Homovc, Federico
 *		ITBA 2011
 *
 *************************************************************************/

/*************************************************************************
 Key Buffer initialization
 ************************************************************************/
void initializeKeyBuffer();

/************************************************************************
 *readKeyboard
 *
 *Reads a buffer of count size from keyboard and puts it on the keybuffer
 ************************************************************************/
void readKeyboard(char* buffer, size_t count);

/************************************************************************
 * clearBuddcopy
 *
 * Fills with zeros the buffcopy global char array
 ***********************************************************************/
void clearBuffcopy(void);

/***********************************************************************
 * deleteCharFromBUff
 *
 * erases a char from the keybuffer and the screen
 *********************************************************************/
void deleteCharFromBuff();

/**********************************************************************
 * addCharToBuff
 *
 * adds a char to the keybuffer and actualize the position of
 * actual_char and the size of the keybuffer
 *********************************************************************/
int addCharToBuff(char c);

/**********************************************************************
 * int_09
 *
 * Each time a interrupt from IRO1 (keyboard) comes across, it
 * selects the correct ascii value and it fills the keyboard
 * buffer with it.
 *
 **********************************************************************/
void int_09();
