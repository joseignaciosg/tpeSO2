/********************************** 
*
*  kernel.c
*  	Galindo, Jose Ignacio
*  	Homovc, Federico
*	Reznik, Luciana
*		ITBA 2011
*
***********************************/


/***	Project Includes	***/
#include "../include/kasm.h"
#include "../include/kernel.h"

extern KEY_BUFFER keybuffer;
DESCR_INT idt[0x90]; /* IDT 144 positions*/
IDTR idtr;			 /* IDTR */



void
initializeIDT()
{
	idtr.base = 0;
	idtr.base +=(dword) &idt;
	idtr.limit = sizeof(idt)-1;
	_lidt (&idtr);
}


void
unmaskPICS(){
	_Cli();
	_mascaraPIC1(0xFC);
   	_mascaraPIC2(0xFF);
	_Sti();
}


void
setup_IDT_entry (DESCR_INT *item, byte selector, dword offset, byte access,
			 byte cero) {
  item->selector = selector;
  item->offset_l = offset & 0xFFFF;
  item->offset_h = offset >> 16;
  item->access = access;
  item->cero = cero;
  
  return;
}


void
reboot(){
	_export( 0x64, 0xFE); /* pulse CPU reset line */
	return;
}

/**********************************************
Starting point of the whole OS
*************************************************/
int
kmain()
{
    /* initializes the key buffer */
	initializeKeyBuffer();

	/* Clears the screen */
	k_clear_screen();


	/* Loads the IDT with the attention routine of IRQ0 */
    setup_IDT_entry (&idt[0x08], 0x08, (dword)&_int_08_hand, ACS_INT, 0);

    /* Loads the IDT with the attention routine of IRQ1 */
    setup_IDT_entry (&idt[0x09], 0x08, (dword)&_int_09_hand, ACS_INT, 0);

    /* Loads the IDT with the attention routine of interrupt 80h */
	setup_IDT_entry (&idt[0x80], 0x08, (dword)&_int_80_hand, ACS_INT, 0);


	/* Loads the IDTR    */
	initializeIDT();

	/* Clears the IMR of both master and slave pics */
	unmaskPICS();

	/* Shows Chinux's splashscreen */
	showSplashScreen();

	wait(5);

	/* Clears the screen */
	k_clear_screen();

	/* calls the shell */
	shell();
	
	return 0;
}

