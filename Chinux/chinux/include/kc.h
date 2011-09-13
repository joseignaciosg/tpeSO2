/**********************
 kc.h
**********************/
#include "defs.h"

#ifndef _kc_
#define _kc_


#define WHITE_TXT 0x07 // video attribute, white letters, black background


/* Clears the screen */
void k_clear_screen();

/* Inicializa la entrada del IDT */
void setup_IDT_entry (DESCR_INT *item, byte selector, dword offset,
					byte access,	 byte cero);

#endif
