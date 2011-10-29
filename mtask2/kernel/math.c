#include <string.h>
#include <dos.h>

#include "kernel.h"

#define CP_SIZE 98
#define EMU_SIZE 0x110
#define CPEMU_SIZE 0x26

static char *cp_data, *emu_data;
extern char far _emu[];

unsigned mt_reserved_stack;

/*
--------------------------------------------------------------------------------
Funciones para guardar/reponer contexto del coprocesador 
--------------------------------------------------------------------------------
*/

#define savecp	db 9bh, 26h, 0ddh, 37h
#define restcp	db 9bh, 26h, 0ddh, 27h

static void
setup_cp(void)
{
	void far *cp_context = cp_data = Malloc(CP_SIZE);
	asm {
		les bx, cp_context
		savecp
	}
}

static void
init_cp(Task_t *task)
{
	memcpy(task->math_data = Malloc(CP_SIZE), cp_data, CP_SIZE);
}

static void
save_cp(Task_t *task)
{
	void far *cp_context = task->math_data;
	
	asm {
		les bx, cp_context
		savecp
	}
}

static void
restore_cp(Task_t *task)
{
	void far *cp_context = task->math_data;
	
	asm {
		les bx, cp_context
		restcp
	}
}

/*
--------------------------------------------------------------------------------
Funciones para inicializar, guardar y reponer contexto del emulador. Solamente
es necesario guardar y reponer contexto en los modelos de datos NEAR.
--------------------------------------------------------------------------------
*/

static void
setup_emu(void)
{
	_fmemcpy(emu_data = Malloc(EMU_SIZE), _emu, EMU_SIZE);
}

#if FAR_DATA

static void
init_emu(Task_t *task)
{
	if ( task->stack )	// NULL para main_task!
		memcpy(task->stack, emu_data, EMU_SIZE);
}

#endif

#if NEAR_DATA

static void
init_emu(Task_t *task)
{
	memcpy(task->math_data = Malloc(EMU_SIZE), emu_data, EMU_SIZE);
}

static void
save_emu(Task_t *task)
{
	_fmemcpy(task->math_data, _emu, EMU_SIZE);
}

static void
restore_emu(Task_t *task)
{
	_fmemcpy(_emu, task->math_data, EMU_SIZE);
}

#endif

/*
--------------------------------------------------------------------------------
Funciones para inicializar contexto del emulador cuando se usa coprocesador,
solamente para modelos de datos FAR.
--------------------------------------------------------------------------------
*/

#if FAR_DATA

static void
setup_cpemu(void)
{
	_fmemcpy(emu_data = Malloc(CPEMU_SIZE), _emu, CPEMU_SIZE);
}

static void
init_cpemu(Task_t *task)
{
	if ( task->stack )	// NULL para main_task!
		memcpy(task->stack, emu_data, CPEMU_SIZE);
}

#endif

/*
--------------------------------------------------------------------------------
Funciones para inicializar, guardar y restaurar el contexto aritmético. En
el modelo de datos NEAR estas funciones se dirigen al coprocesador, si existe,
o al emulador en caso contrario.
En los modelos de datos FAR, si existe coprocesador se dirigen al coprocesador, 
pero cuando se crea una tarea también debe inicializarse en su stack una parte 
de datos constantes que requiere el emulador. En este modelo no es necesario
guardar y restaurar contexto del emulador, pues cada tarea tiene un segmento de
stack separado.
El emulador guarda sus datos en la parte inferior del segmento de stack,de modo
que es necesario tomar esto en cuenta cuando se crea el stack de una tarea.
--------------------------------------------------------------------------------
*/

#if FAR_DATA

void
mt_setup_math(void)
{
	if ( _8087 )
	{
		setup_cp();
		setup_cpemu();
		mt_reserved_stack = CPEMU_SIZE;
	}
	else
	{
		setup_emu();
		mt_reserved_stack = EMU_SIZE;
	}
}

void
mt_init_math(Task_t *task)
{
	if ( _8087 )
	{
		init_cp(task);
		init_cpemu(task);
	}
	else
		init_emu(task);
}

void
mt_save_math(Task_t *task)
{
	save_cp(task);
}

void
mt_restore_math(Task_t *task)
{
	restore_cp(task);
}

#endif

#if NEAR_DATA

void
mt_setup_math(void)
{
	if ( _8087 )
		setup_cp();
	else
		setup_emu();
}

void
mt_init_math(Task_t *task)
{
	if ( _8087 )
		init_cp(task);
	else
		init_emu(task);
}

void
mt_save_math(Task_t *task)
{
	if ( _8087 )
		save_cp(task);
	else
		save_emu(task);
}

void
mt_restore_math(Task_t *task)
{
	if ( _8087 )
		restore_cp(task);
	else
		restore_emu(task);
}

#endif

