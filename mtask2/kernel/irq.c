#include <stdlib.h>
#include <stdio.h>
#include <dos.h>
#include <conio.h>

#include "kernel.h"

#define ISR_STACK	4096
#define MASTER		0x20
#define SLAVE		0xa0
#define EOI			0x20
#define MASK(pic)	((pic) + 1)
#define	BIT(n)		(1 << ((n) & 7))

typedef void interrupt (*IntVector_t)(void);

typedef struct
{
	unsigned vec_num;
	IntVector_t old_vector;
	IntVector_t new_vector;
	Handler_t isr;
}
IntControl_t;

unsigned mt_int_level;
unsigned mt_irq_number;

static char isr_stack[ISR_STACK];
static unsigned char old_mask_master, old_mask_slave;

static IntControl_t int_control[16] = /*para capturar interrupciones*/
{
	0x08,	NULL,	mt_hw_vec_0,	NULL,
	0x09,	NULL,	mt_hw_vec_1,	NULL,
	0x0a,	NULL,	mt_hw_vec_2,	NULL,
	0x0b,	NULL,	mt_hw_vec_3,	NULL,
	0x0c,	NULL,	mt_hw_vec_4,	NULL,
	0x0d,	NULL,	mt_hw_vec_5,	NULL,
	0x0e,	NULL,	mt_hw_vec_6,	NULL,
	0x0f,	NULL,	mt_hw_vec_7,	NULL,
	0x70,	NULL,	mt_hw_vec_8,	NULL,
	0x71,	NULL,	mt_hw_vec_9,	NULL,
	0x72,	NULL,	mt_hw_vec_10,	NULL,
	0x73,	NULL,	mt_hw_vec_11,	NULL,
	0x74,	NULL,	mt_hw_vec_12,	NULL,
	0x75,	NULL,	mt_hw_vec_13,	NULL,
	0x76,	NULL,	mt_hw_vec_14,	NULL,
	0x77,	NULL,	mt_hw_vec_15,	NULL
};

/*
--------------------------------------------------------------------------------
Vector generico para capturar interrupciones de hardware.

El vector generico mantiene una variable mt_int_level que lleva cuenta del 
nivel de anidamiento de las interrupciones. Esta variable es 0 si no se esta
dentro de una interrupcion. Cuando se produce una interrupcion de primer nivel
(mt_int_level pasa de 0 a 1) se cambia a un stack interno. Si se producen
interrupciones anidadas, se sigue utilizando el mismo stack. El stack se repone
cuando termina la interrupcion de primer nivel (mt_int_level pasa de 1 a 0),
pero antes se llama a la funcion mt_select_task() que elige la proxima tarea
a ejecutar (posiblemente modificando mt_curr_task).
Esta funcion es llamada desde los vectores mt_hw_vec_NN() correspondientes a
cada IRQ que estan en el archivo hardvec.asm, pasandole el numero de IRQ en
la variable global mt_irq_number; esto es seguro porque se hace con 
interrupciones deshabilitadas.
--------------------------------------------------------------------------------
*/

#if FAR_DATA
	#define ISR_SS		FP_SEG(isr_stack)
	#define ISR_SP		even(FP_OFF(&isr_stack[ISR_STACK]))
#else
	#define ISR_SS		_DS
	#define ISR_SP		even((unsigned)&isr_stack[ISR_STACK])
#endif

void interrupt
mt_hw_vector(void)
{
	if ( !mt_int_level++ )/*todas las interrupciones anidadas se van a manejar con un stack interno del kernel*/
	{
		mt_curr_task->ss = _SS;
		mt_curr_task->sp = _SP;
		_SS = ISR_SS;
		_SP = ISR_SP;
	}

	int_control[mt_irq_number].isr(mt_irq_number);
	disable();

	if ( !--mt_int_level )
	{
		mt_select_task();
		_SS = mt_curr_task->ss;
		_SP = mt_curr_task->sp;
	}
}

/*
--------------------------------------------------------------------------------
Inicializacion y cleanup del modulo.
--------------------------------------------------------------------------------
*/

void
mt_restore_irqs(void)
{
	int i;

	DisableInts();
	outp(MASK(MASTER), old_mask_master);
	outp(MASK(SLAVE), old_mask_slave);
	for ( i = 0 ; i < 16 ; i++ )
		SetHandler(i, NULL);
	RestoreInts();
}

void
mt_setup_irqs(void)
{	
	int i;
	IntControl_t *ctl;

	old_mask_master = inp(MASK(MASTER));
	old_mask_slave = inp(MASK(SLAVE));
}

/*
--------------------------------------------------------------------------------
SetHandler - establece un manejador de interrupcion de hardware.

Si se esta capturando el vector de interrupcion, guarda el vector original
para llamar desde OldInterrupt().
--------------------------------------------------------------------------------
*/

void
SetHandler(unsigned irq, Handler_t isr)
{
	IntControl_t *ctl = &int_control[irq];
	
	DisableInts();
	if ( !ctl->isr && isr )
	{
		ctl->old_vector = getvect(ctl->vec_num);
		setvect(ctl->vec_num, ctl->new_vector);
	}
	else if ( ctl->isr && !isr )
		setvect(ctl->vec_num, ctl->old_vector);
	ctl->isr = isr;
	RestoreInts();
}

/*
--------------------------------------------------------------------------------
DisableIRQ, EnableIRQ - habilitacion y deshabilitacion de IRQs.
--------------------------------------------------------------------------------
*/

void
DisableIRQ(unsigned irq)
{
	DisableInts();
	if ( irq <= 7 )
		outp(MASK(MASTER), inp(MASK(MASTER)) | BIT(irq));
	else
		outp(MASK(SLAVE), inp(MASK(SLAVE)) | BIT(irq));
	RestoreInts();
}

void
EnableIRQ(unsigned irq)
{
	DisableInts();
	if ( irq <= 7 )
		outp(MASK(MASTER), inp(MASK(MASTER)) & ~BIT(irq));
	else
		outp(MASK(SLAVE), inp(MASK(SLAVE)) & ~BIT(irq));
	RestoreInts();
}

/*
--------------------------------------------------------------------------------
EndInterrupt - fin de una interrupcion de hardware.

Envia EOI al PIC, debe utilizarse antes de retornar de un manejador de
interrupcion si no se llamo al manejador original, en caso contrario este
IRQ queda deshabilitado permanentemente.
--------------------------------------------------------------------------------
*/

void
EndInterrupt(unsigned irq)
{
	if ( irq > 7 )					/* dar EOI al PIC esclavo */
		outp(SLAVE, EOI);
	outp(MASTER, EOI);				/* dar EOI al PIC principal */
}

/*
--------------------------------------------------------------------------------
OldInterrupt - llama al vector original de una interrupcion.
--------------------------------------------------------------------------------
*/

void
OldInterrupt(unsigned irq)
{
	int_control[irq].old_vector();
}
