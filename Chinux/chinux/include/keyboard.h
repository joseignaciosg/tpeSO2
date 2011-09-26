/***************************************************
*  utils.h
*  Keyboard Handling Functions
*  	Galindo, Jose Ignacio
*  	Homovc, Federico
*	Reznik, Luciana
*		ITBA 2011
*
****************************************************/


/*
 * inb
 *
 * Is called by  _int_09_hander each time a keyboard interrupt
 * comes across.
 * Returns the make and the break code of the pressed key.
 */
static inline unsigned char inb( unsigned short port );

/*
 * int_09
 *
 * Each time a interrupt from IRO1 (keyboard) comes across, it
 * selects the correct asci value and it fills the keyboard
 * buffer with it.
 *
 */
void int_09();

/*
 * getInstruction
 *
 * Copies the ketboard buffer into buffcopy global char array
 * and comletes it with a -1
 */
void getInstruction(void);

/*
 * clearBuddcopy
 *
 * Fills with ceros the buffcopy globar char array
 */
void clearBuffcopy(void);

/*
 * buffcopy - DEPRECATED
 *
 * copies the actual keyboard buffer to the globlal char array buffcopy
 * */
void buffercopy();

void initializeKeyBuffer();

void readKeyboard(char* buffer, size_t count);

