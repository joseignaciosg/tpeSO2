/********************************** 
 *
 *  stdio.h
 *  	Galindo, Jose Ignacio
 *  	Homovc, Federico
 *		Reznik, Luciana
 *		ITBA 2011
 *
 ***********************************/

#ifndef _stdio_
#define _stdio_

/**********************************************************************************************************
 * putc
 *
 * Prints a character into the standard output stream
 ************************************************************************************************************/
void putc(char c);

/**********************************************************************************************************
 *printstr
 *
 *Prints a string in the standard output stream
 *Is used by printf
 **********************************************************************************************************/
int printstr(char * s);

/***************************************************************************
 * clearc
 *
 * Erases a character from STDOUT
 ***************************************************************************/
void clearc(char c);

/***********************************************************************
 * getc
 *
 * Gets a character form STDUOT
 ************************************************************************/
char getc();

/***********************************************************************
 * int_80
 *
 *is called from _int_80_handler and is used to select the driver for
 *the selected stream.
 *
 **********************************************************************/
void int_80(size_t call, size_t fd, char *buffer, size_t count);

/*********************************************************************
 *  __write
 *
 * Primitive of the IO system
 * It calls the _int_80_caller routine , and is the
 * point of connection of the system with the kernel for
 * writing something into a stream
 *
 ***********************************************************************/
size_t __write(size_t call, size_t fd, void* buffer, size_t count);

/**********************************************************************
 * __read
 *
 * Primitive of the IO system
 * It calls the _int_80_caller routine , and is the
 * point of connection of the system with the kernel for
 * reading something from a stream
 *
 ********************************************************************/
size_t __read(size_t fd, void* buffer, size_t count);

/*******************************************************************
 * printftest
 *
 * Converts, gives format, and prints its argument
 * in STDOUT
 ********************************************************************/
int printf(char * format, ...);

/*******************************************************************
 *scanf
 *
 * Reads a number of arguments from STDIN with a given
 * format and saves them in each of those arguments.
 ********************************************************************/
int scanf(char* format, ...);

/*********************************************************************
 * scanfs
 *
 * Scans a String from origin and put it on dest
 * Return the quantity of written chars
 * Is called by scanf
 *********************************************************************/
int scanfs(char* dest, char* origin);

/**********************************************************************
 * scanfi
 *
 * Scans an integer form origin and put it on dest
 * Returns the length of the number
 * Is called by scanf
 *********************************************************************/
int scanfi(int* dest, char* origin);

/***********************************************************************
 *
 *
 * Scans a double from origin and put it on dest.
 * Returns the position of the dot that separates the
 * integer part from the fraction in the double.
 * Is called by scanf
 **********************************************************************/
int scanfd(double* dest, char* origin);

#endif
