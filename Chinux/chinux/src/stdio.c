/********************************** 
 *
 *  stdio.c
 *  	Galindo, Jose Ignacio
 *  	Homovc, Federico
 *		Reznik, Luciana
 *		ITBA 2011
 *
 ***********************************/

/***	Project Includes	***/
#include "../include/defs.h"
#include "../include/utils.h"
#include "../include/stdio.h"

/***	System Includes		***/
#include "stdarg.h"

extern KEY_BUFFER keybuffer;
extern unsigned int timestick; //for timer tick
extern char buffcopy[BUFFER_SIZE];

void
putc(char c) {
	void *p = &c;
	if (c == '\n')
		enter();
	else
		__write(WRITE, STDOUT, p, 1);
	return;
}

int
printstr(char * s) {
	unsigned int i = 0;
	while (s[i]) {
		putc(s[i++]);
	}
	return i;
}

void
clearc(char c) {
	void *p = &c;
	__write(ERASE, STDOUT, p, 1);
	return;
}


char getc() {
	char c;
	void *p = &c;
	__read(STDIN, p, 1);
	return *(char*) p;
}


void int_80(size_t call, size_t fd, char *buffer, size_t count) {
	unsigned int j = 0;
	switch (call) {
	case WRITE:
		if (fd == STDOUT) {
			writeScreen(buffer, count);
		}
		break;

	case ERASE:
		if (fd == STDOUT) {
			eraseScreen(buffer, count);
		}
		break;
	case READ:
		if (fd == STDIN) {
			readKeyboard(buffer, count);
		}
		break;
	default:
		break;
	}

	return;
}


size_t
__write(size_t call, size_t fd, void* buffer, size_t count) {
	switch (call) {
	case WRITE:
		if (fd == STDOUT){
			_int_80_caller(WRITE, STDOUT, buffer, count);
		}
		break;
	case ERASE:
		if (fd == STDOUT){
			_int_80_caller(ERASE, STDOUT, (char*) buffer, count);
		}
		break;
	}

	return count;
}


size_t
__read(size_t fd, void* buffer, size_t count) {
	if (fd == STDIN) {
		_int_80_caller(READ, STDIN, (char*) buffer, count);
	}
	return count;
}


int
printf(char * format, ...) {
	int count = 0;
	char *p;
	va_list ap;
	va_start(ap,format);

	for (p = format; *p; p++) {
		count++;
		if (*p != '%') {
			putc(*p);
			continue;
		}
		switch (*++p) {
		case 'l':
			if (*(p + 1) == 'd') {
				count += ltoa(va_arg(ap, unsigned long int));
				p++;
			}
			break;
		case 'd':
			count += itoa(va_arg(ap,int));
			break;
		case 'f':
			count += ftoa(va_arg(ap,double));
			break;
		case 's':
			count += printstr(va_arg(ap,char*));
			break;
		}
	}

	va_end(ap);

	return count;
}


int
scanf(char* format, ...) {
	int count = 0;
	char *p;
	int i = 0;
	va_list ap;
	va_start(ap,format);
	__read(STDIN, buffcopy, keybuffer.size);
	for (p = format; *p; p++) {
		if (*p != '%') {
			continue;
		}
		switch (*++p) {
		case 's':
			count = scanfs(va_arg(ap,char*), buffcopy);
			break;
		case 'd':
			count = scanfi(va_arg(ap,int*), buffcopy);
			break;
		case 'f':
			count = scanfd(va_arg(ap,double*), buffcopy);
			break;
		}
	}
	va_end(ap);

	return count;
}


int
scanfs(char* dest, char* origin) {
	int count;
	for (count = 0; origin[count]; count++)
		dest[count] = origin[count];
	dest[count] = 0;

	return count;
}


int
scanfi(int* dest, char* origin) {
	int aux = 0, count, length, pow = 1, negative = FALSE, maxCount = 0;
	if (origin[0] == '-') {
		negative = TRUE;
		maxCount = 1;
	}
	count = str_len(origin) - 1;
	length = count;
	for (; count >= maxCount; count--) {
		aux += ((origin[count] - '0') * pow);
		pow *= 10;
	}
	if (negative)
		aux = -aux;
	*(dest) = aux;

	return length + 1;
}


int scanfd(double* dest, char* origin) {
	int count, pointPos = 0, isDecimal = FALSE, negative = FALSE, maxCount = 0;
	double aux = 0, pow = 1;

	if (origin[0] == '-') {
		negative = TRUE;
		maxCount = 1;
	}

	while (origin[pointPos] && origin[pointPos] != '.') {
		++pointPos;
		if (origin[pointPos] == '.')
			isDecimal = TRUE;
	}

	for (count = pointPos - 1; count >= maxCount; count--) {
		aux += ((origin[count] - '0') * pow);
		pow *= 10;
	}

	count = pointPos + 1;
	pow = 0.1;
	while (origin[count] && isDecimal) {
		aux += ((origin[count++] - '0') * pow);
		pow /= 10;
	}
	if (negative)
		aux = -aux;
	*(dest) = aux;

	return pointPos;
}

