/********************************** 
*
*  kasm.h
*  	Galindo, Jose Ignacio
*  	Homovc, Federico
*  	Loreti, Nicolas
*		ITBA 2011
*
***********************************/

#include "defs.h"


unsigned int    _read_msw();

void            _lidt (IDTR *idtr);

void		_mascaraPIC1 (byte mascara);  /* Escribe mascara de PIC1 */

void		_mascaraPIC2 (byte mascara);  /* Escribe mascara de PIC2 */

void		_Cli(void);        /* Deshabilita interrupciones  */

void		_Sti(void);	 /* Habilita interrupciones  */

void		_int_08_hand();      /* Timer tick */

void		_int_09_hand();      /* Keyboard Interrupt */

void		_int_80_hand();      /* Basic IO Interrupt */

void		_int_79_hand();      /*  Interrupt for Kill and Block */

void		_debug (void);

