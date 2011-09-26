/********************************** 
*
*  stdio.h
*  	Galindo, Jose Ignacio
*  	Homovc, Federico
*	Reznik, Luciana
*		ITBA 2011
*
***********************************/

#ifndef _stdio_
#define _stdio_

/* __write
*
* Recibe como parametros:
* - File Descriptor
* - Buffer del source
* - Cantidad
*
**/
size_t __write(size_t fd, void* buffer,size_t count);


/* __read
*
* Recibe como parametros:
* - File Descriptor
* - Buffer a donde escribir
* - Cantidad
*
**/
size_t __read(size_t fd, void* buffer, size_t count);

/* Shows the initial screen */
void showSplashScreen();

/* Tiempo de espera */
void wait(double time);

void putc(char c);

int printstr( char * s);

void clearc(char c);

char getc();

void scrolldown();

int printf( char * format, ...);

int scanf(char* format, ...);

void print(char s[]);

int scanfs(char*dest, char*origin);

int scanfi(int*dest, char*origin);

int scanfd(double*dest, char*origin);

/*
 * int_80
 *
 *Is called by __write and __read primitives and it is used for
 *writing something on the specified file descriptor (fd).
 *
 */
void int_80(size_t call,size_t fd, char *buffer, size_t count);

#endif
