/********************************** 
*
*  video.c
*  	Galindo, Jose Ignacio
*  	Homovc, Federico
*	Reznik, Luciana
*		ITBA 2011
*
***********************************/

/***	Project Includes	***/
#include "../include/defs.h"
#include "../include/video.h"
#include "../include/kc.h"

/*points to the videoboard */
char * vidmem = (char *) 0xb8000;
unsigned int curpos = 0;

void
k_clear_screen()
{
	unsigned int i=0;
	while(i < (80*25*2))
	{
		vidmem[i]=' ';
		i++;
		vidmem[i]=WHITE_TXT; //0x68
		i++;
	}
	curpos=0;
	return;
}


void
writeScreen(char* buffer, int count)
{
	int j = 0;

	while ( j < count )
	{
		if(curpos == 80*25*2){
			scrolldown();
		}
		vidmem[curpos] = buffer[j];
		curpos++;
		vidmem[curpos] = WHITE_TXT;
		curpos++;
		j++;
	}
	return;
}


void
eraseScreen(char* buffer, int count)
{
	int j = 0;
	if (curpos > 0) {
		while (j < count) {
			curpos--;
			curpos--;
			vidmem[curpos] = buffer[j];
			j++;
		}
	}
	return;
}




void
scrolldown()
{
	unsigned int i = 0;
	while(i < (80 * 2 * 25))
	{
		vidmem[i]=vidmem[i+80*2];
		i++;
		vidmem[i]=WHITE_TXT;
		i++;
	};
	curpos=(80*24*2);

	return;
}


void
enter()
{
	if(curpos > 80*24*2)
		scrolldown();
	else
		curpos += 80*2 - curpos%(80*2);

	return;
}



void 
moveCursor(){
	_Cli();
	_export(0x3D4, 0x0F);
	_export(0x3D5, (curpos/2) & 0xFF);
	_export(0x3D4, 0x0E);
	_export(0x3D5, ((curpos/2)>>8) & 0xFF);
	_Sti();

	return;
 }

