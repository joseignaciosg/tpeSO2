/********************************** 
*
*  video.c
*  	Galindo, Jose Ignacio
*  	Homovc, Federico
*	Loreti, Nicolas
*		ITBA 2011
*
***********************************/

/***	Project Includes	***/
#include "../include/defs.h"
#include "../include/video.h"

/*points to the videoboard */
char * vidmem = (char *) 0xb8000;
extern int currentTTY;
extern int currentProcessTTY;
extern TTY terminals[4];
extern int keypressed;

void
k_clear_screen()
{
	int i = 0;
	while(i < (80*25*2))
	{
		if(currentProcessTTY == currentTTY)
			vidmem[i]=' ';
		terminals[currentProcessTTY].terminal[i] = ' ';
		i++;
		if(currentProcessTTY == currentTTY)
			vidmem[i] = WHITE_TXT;
		terminals[currentProcessTTY].terminal[i] = WHITE_TXT;
		i++;
	}
	terminals[currentProcessTTY].curpos = 0;
	return;
}


void
writeScreen(char* buffer, int count)
{
	int j = 0;

	while ( j < count )
	{
		if(keypressed)
		{
			if(terminals[currentTTY].curpos == 80*25*2)
				scrolldown();
			vidmem[terminals[currentTTY].curpos] = buffer[j];
			terminals[currentTTY].curpos++;
			vidmem[terminals[currentTTY].curpos] = WHITE_TXT;
			terminals[currentTTY].curpos++;
			j++;
		}
		else
		{
			if(terminals[currentProcessTTY].curpos == 80*25*2)
				scrolldown();
			
			if(currentProcessTTY == currentTTY)
				vidmem[terminals[currentTTY].curpos] = buffer[j];
			terminals[currentProcessTTY].terminal[terminals[currentProcessTTY].curpos] = buffer[j];
			terminals[currentProcessTTY].curpos++;
			if(currentProcessTTY == currentTTY)
				vidmem[terminals[currentTTY].curpos] = WHITE_TXT;
			terminals[currentProcessTTY].terminal[terminals[currentProcessTTY].curpos] = WHITE_TXT;
			terminals[currentProcessTTY].curpos++;
			j++;
		}
	}
	return;
}


void
eraseScreen(char* buffer, int count)
{
	int j = 0;
	if (terminals[currentTTY].curpos > 0) {
		while (j < count) {
			terminals[currentTTY].curpos -= 2;
			vidmem[terminals[currentTTY].curpos] = buffer[j];
			terminals[currentTTY].terminal[terminals[currentTTY].curpos] = buffer[j];
			j++;
		}
	}
	return;
}


void
scrolldown()
{
	int i = 0;
	while(i < (80 * 2 * 24))
	{
		if(currentProcessTTY == currentTTY)
			vidmem[i] = vidmem[i + 80 * 2];
		terminals[currentProcessTTY].terminal[i] = terminals[currentProcessTTY].terminal[i + 80 * 2];
		i++;
		if(currentProcessTTY == currentTTY)
			vidmem[i] = WHITE_TXT;
		terminals[currentProcessTTY].terminal[i] = WHITE_TXT;
		i++;
	}
	while(i < (80 * 2 * 25))
	{
		if(currentProcessTTY == currentTTY)
			vidmem[i] = ' ';
		terminals[currentProcessTTY].terminal[i] = ' ';
		i++;
		if(currentProcessTTY == currentTTY)
			vidmem[i] = WHITE_TXT;
		terminals[currentProcessTTY].terminal[i] = WHITE_TXT;
		i++;
	}
	terminals[currentProcessTTY].curpos = (80 * 24 * 2);

	return;
}


void
enter()
{
	if(terminals[currentProcessTTY].curpos > 80*24*2)
		scrolldown();
	else
		terminals[currentProcessTTY].curpos += 80 * 2 - terminals[currentProcessTTY].curpos % (80 * 2);

	return;
}



void 
moveCursor(){
	if(currentProcessTTY == currentTTY)
	{
		_export(0x3D4, 0x0F);
		_export(0x3D5, (terminals[currentTTY].curpos/2) & 0xFF);
		_export(0x3D4, 0x0E);
		_export(0x3D5, ((terminals[currentTTY].curpos/2)>>8) & 0xFF);
	}

	return;
 }

