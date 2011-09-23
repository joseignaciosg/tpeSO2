#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <signal.h>
#include <alloc.h>
#include <string.h>

#include "kernel.h"

#define CLOCKIRQ		0				/* interrupcion de timer */
#define NULL_STACK		256 			/* tamano de stack para proceso nulo */ 
#define INIFL			0x200
#define QUANTUM			2
#define MSPERTICK 		55

Task_t *mt_curr_task;					/* proceso en ejecucion */

static Task_t *last_task;				/* proceso anterior */
static Task_t main_task;				/* proceso principal */
static Time_t ticks_to_run;				/* ranura de tiempo */
static TaskQueue_t ready_q;				/* cola de procesos ready */
static TaskQueue_t terminated_q;		/* cola de procesos terminados */
static Switcher_t save_restore;			/* cambio de contexto adicional */

static void init_kernel(void);			/* inicializacion */
static void end_kernel(void);			/* finalizacion */

#ifdef HIREGS
static bool use_hiregs;					/* hay registros de 32 bits */
static void check_hiregs(void);			/* hay registros de 32 bits? */
static void save_hiregs(Task_t *task);
static void restore_hiregs(Task_t *task);
#endif

static void scheduler(void);
static void interrupt context_switch(void);

static void block(Task_t *task, TaskState_t state);
static void ready(Task_t *task, bool success);
static void free_task(Task_t *task);

static Time_t msecs_to_ticks(Time_t msecs);
static Time_t ticks_to_msecs(Time_t ticks);

static void free_terminated(void);		/* libera tareas terminadas */
static void do_nothing(void *arg);		/* funcion del proceso nulo */
static void clockint(unsigned irq);		/* manejador interrupcion de timer */

typedef struct
{
	InterruptRegs_t	regs; /*estructura que tiene todos los registros en la forma en que uno los pushea*/
	void			(*retaddr)(void);/*direcci—n de retorno de una funci—n que lo liquida*/
	void *			arg;/*argumento*/
}
InitialStack_t; /*stack inicial que debe crear la funcion de creaci—n procesos*/

/*
--------------------------------------------------------------------------------
Malloc, Realloc, StrDup, Free - manejo de memoria dinamica
--------------------------------------------------------------------------------
*/

void *
Malloc(unsigned size)
{
	void *p;

	Atomic();/*se sale del modo preemrivo*/
	free_terminated();/*recoleci—n de basura*/
	if ( !(p = malloc(size)) )
		Panic("Error malloc");
	memset(p, 0, size);
	Unatomic();/*se vuelve el modo preemtivo*/
	return p;
}

void *
Realloc(void *mem, unsigned size)
{
	void *p;

	if ( !mem )
		return Malloc(size);
	Atomic();
	free_terminated();
	if ( !(p = realloc(mem, size)) )
		Panic("Error realloc");
	Unatomic();
	return p;
}

char *
StrDup(char *str)
{
	char *p;

	if ( !str )
		return NULL;
	Atomic();
	free_terminated();
	if ( !(p = strdup(str)) )
		Panic("Error strdup");
	Unatomic();
	return p;
}

void
Free(void *mem)
{
	if ( !mem )
		return;
	Atomic();
	free(mem);
	Unatomic();
}

/*
--------------------------------------------------------------------------------
msecs_to_ticks, ticks_to_msecs - conversion de milisegundos a ticks y viceversa
--------------------------------------------------------------------------------
*/

static Time_t 
msecs_to_ticks(Time_t msecs)
{
	return (msecs + MSPERTICK - 1) / MSPERTICK;
}

static Time_t 
ticks_to_msecs(Time_t ticks)
{
	return ticks * MSPERTICK;
}

/*
--------------------------------------------------------------------------------
block - bloquea un proceso
--------------------------------------------------------------------------------
*/

static void
block(Task_t *task, TaskState_t state) /*lo saca de toda cola, 
										lo pone en derterminado estado de bloquo, y fuera de toda cola*/
{
	mt_dequeue(task);
	mt_dequeue_time(task);
	task->state = state;
}

/*
--------------------------------------------------------------------------------
ready - desbloquea un proceso y lo pone en la cola de ready

Si el proceso estaba bloqueado en WaitQueue, Send o Receive, el argumento
success determina el status de retorno de la funcion que lo bloqueo.
--------------------------------------------------------------------------------
*/

static void
ready(Task_t *task, bool success)
{
	if ( task->state == TaskReady )
		return;

	mt_dequeue(task);
	mt_dequeue_time(task);
	mt_enqueue(task, &ready_q);
	task->success = success;
	task->state = TaskReady;
}

/*
--------------------------------------------------------------------------------
CreateTask - crea un proceso.

Recibe un puntero a una funcion de tipo void f(void*), tamano del stack,
un puntero para pasar como argumento, nombre del proceso, prioridad inicial y
si el proceso usa el coprocesador aritmetico.
Toma memoria para crear el stack del proceso y lo inicializa para que retorne
a Exit(). Una vez creado un proceso, hay que comenzar a ejecutarlo llamando 
a Ready().
--------------------------------------------------------------------------------
*/

Task_t *
CreateTask(TaskFunc_t func, unsigned stacksize, void *arg,
						char *name, Prio_t priority)
{
	Task_t *task;
	InitialStack_t *s;

	/* ajustar tamano del stack y redondear a numero par (el stack a de a dos bytes) */
	stacksize = even(stacksize + mt_reserved_stack);

	/* alocar estructura del proceso */
	task = Malloc(sizeof(Task_t));
	task->name = task->send_queue.name = StrDup(name);
	task->priority = priority;
	task->stack = Malloc(stacksize);

	/* inicializar stack */
	s = (InitialStack_t *)(task->stack + stacksize) - 1;
	s->arg = arg;
	s->retaddr = Exit;				/* direccion de retorno de f() */
	s->regs.x.flags = INIFL;

#if FAR_CODE
	s->regs.x.cs = FP_SEG(func);
	s->regs.x.ip = FP_OFF(func);	/* simula interrupcion al entrar a func() */
#else
	s->regs.x.cs = _CS;
	s->regs.x.ip = (unsigned) func;
#endif
	s->regs.x.ds = _DS;

#if FAR_DATA /*NO DARLE BOLA A ESTO*/
	task->ss = FP_SEG(s);
	task->sp = FP_OFF(s);
#else
	task->ss = _DS;
	task->sp = (unsigned) s;
#endif

	return task;
}

/*
--------------------------------------------------------------------------------
ProtectMath - inicializa contexto artimetico para una tarea

Debe ser llamada para todas las tareas que utilicen punto flotante.
--------------------------------------------------------------------------------
*/

void
ProtectMath(Task_t *task)
{
	if ( !task->math_data )
		mt_init_math(task);
}

/*
--------------------------------------------------------------------------------
DeleteTask - elimina un proceso creado con CreateTask

Si es el proceso actual, envia un mensaje al proceso de limpieza y se bloquea
como terminado.
--------------------------------------------------------------------------------
*/

static void
free_task(Task_t *task)
{
	free(task->name);
	free(task->stack);
	if ( task->math_data )
		free(task->math_data);
	free(task);
}

void
DeleteTask(Task_t *task)
{
	if ( task == &main_task )
		Panic("Imposible eliminar el proceso principal");

	FlushQueue(&task->send_queue, false);
	DisableInts();
	if ( task == mt_curr_task )
	{
		mt_curr_task->state = TaskTerminated;
		mt_enqueue(mt_curr_task, &terminated_q);/*no podemos liberar la memoria todav’a por que estamos
												 usando el stack*/
		scheduler();
	}
	else
	{
		block(task, TaskTerminated);/*si no esta ejecutando, libero la el bcp(bloque de control de procesos)
									 y el stack*/
		free_task(task);
	}
	RestoreInts();
}

/*
--------------------------------------------------------------------------------
free_terminated - elimina las tareas terminadas.
--------------------------------------------------------------------------------
*/

void
free_terminated(void)/*garbage collector*/
{
	Task_t *task;

	while ( task = mt_getlast(&terminated_q) )
		free_task(task);
}

/*
--------------------------------------------------------------------------------
CurrentTask - retorna un puntero al proceso actual.
--------------------------------------------------------------------------------
*/

Task_t *
CurrentTask(void)
{
	return mt_curr_task;
}


/*
--------------------------------------------------------------------------------
Panic - error fatal del sistema
--------------------------------------------------------------------------------
*/

void
Panic(char *msg)
{
	Atomic();
	cprintf("\r\n[%s]: %s\r\n", mt_curr_task->name, msg);
	exit(1);
}

/*
--------------------------------------------------------------------------------
Pause - suspende el proceso actual
--------------------------------------------------------------------------------
*/

void
Pause(void)
{
	Suspend(mt_curr_task);
}

/*
--------------------------------------------------------------------------------
Yield - cede voluntariamente la CPU
--------------------------------------------------------------------------------
*/

void
Yield(void)
{
	Ready(mt_curr_task);/*lo vuelve a poner en la cola de listos*/
}

/*
--------------------------------------------------------------------------------
Delay - pone al proceso actual a dormir durante una cantidad de milisegundos
--------------------------------------------------------------------------------
*/

void
Delay(Time_t msecs)
{
	DisableInts();
	if ( msecs )
	{
		block(mt_curr_task, TaskDelaying);
		if ( msecs != FOREVER )
			mt_enqueue_time(mt_curr_task, msecs_to_ticks(msecs));
	}
	else
		ready(mt_curr_task, false);
	scheduler();
	RestoreInts();
}

/*
--------------------------------------------------------------------------------
Exit - finaliza el proceso actual

Todos los procesos creados con CreateTask retornan a esta funcion que los mata.
Esta funcion nunca retorna.
--------------------------------------------------------------------------------
*/

void
Exit(void)
{
	DeleteTask(mt_curr_task);
}

/*
--------------------------------------------------------------------------------
Suspend - suspende un proceso
--------------------------------------------------------------------------------
*/

void
Suspend(Task_t *task)
{
	DisableInts();
	block(task, TaskSuspended);
	if ( task == mt_curr_task )
		scheduler();
	RestoreInts();
}

/*
--------------------------------------------------------------------------------
Ready - pone un proceso en la cola ready
--------------------------------------------------------------------------------
*/

void  
Ready(Task_t *task)
{
	DisableInts();
	ready(task, false);
	scheduler();
	RestoreInts();
}

/*
--------------------------------------------------------------------------------
GetPriority - retorna la prioridad de un proceso
--------------------------------------------------------------------------------
*/

Prio_t
GetPriority(Task_t *task)
{
	return task->priority;
}

/*
--------------------------------------------------------------------------------
SetPriority - establece la prioridad de un proceso

Si el proceso estaba en una cola, lo desencola y lo vuelve a encolar para
reflejar el cambio de prioridad en su posición en la cola.
Si se le ha cambiado la prioridad al proceso actual o a uno que esta ready se
llama al scheduler.
--------------------------------------------------------------------------------
*/

void		
SetPriority(Task_t *task, Prio_t priority)
{
	TaskQueue_t *queue;

	DisableInts();
	task->priority = priority;
	if ( queue = task->queue )
	{
		mt_dequeue(task);
		mt_enqueue(task, queue);
	}
	if ( task == mt_curr_task || task->state == TaskReady )
		scheduler();
	RestoreInts();
}

/*
--------------------------------------------------------------------------------
SetData - establece un puntero a datos privados de un proceso
--------------------------------------------------------------------------------
*/

void		
SetData(Task_t *task, void *data)
{
	DisableInts();
	task->data = data;
	RestoreInts();
}

/*
--------------------------------------------------------------------------------
SetSwitcher - establece un manejador global de cambio de contexto

Si existe este manejador, esta funcion se llamara inmediatamente antes de cada
cambio de contexto. Recibe un puntero a la tarea que deja la CPU y otro a la
tarea que la recibe. Se ejecuta con interrupciones deshabilitadas y no debe
habilitarlas.
--------------------------------------------------------------------------------
*/

void		
SetSwitcher(Switcher_t switcher)
{
	DisableInts();
	save_restore = switcher;
	RestoreInts();
}

/*
--------------------------------------------------------------------------------
CreateQueue - crea una cola de procesos
--------------------------------------------------------------------------------
*/

TaskQueue_t *	
CreateQueue(char *name)
{
	TaskQueue_t *queue = Malloc(sizeof(TaskQueue_t));

	queue->name = StrDup(name);
	return queue;
}

/*
--------------------------------------------------------------------------------
DeleteQueue - destruye una cola de procesos
--------------------------------------------------------------------------------
*/

void
DeleteQueue(TaskQueue_t *queue)
{
	FlushQueue(queue, false);
	Free(queue->name);
	Free(queue);
}

/*
--------------------------------------------------------------------------------
WaitQueue, WaitQueueCond, WaitQueueTimed - esperar en una cola de procesos

El valor de retorno es true si el proceso fue despertado por SignalQueue
o el valor pasado a FlushQueue.
Si msecs es FOREVER, espera indefinidamente. Si msecs es cero, retorna false.
--------------------------------------------------------------------------------
*/

bool			
WaitQueue(TaskQueue_t *queue)
{
	return WaitQueueTimed(queue, FOREVER);
}

bool			
WaitQueueTimed(TaskQueue_t *queue, Time_t msecs)
{
	bool success;

	if ( !msecs )
		return false;

	DisableInts();
	block(mt_curr_task, TaskWaiting);
	mt_enqueue(mt_curr_task, queue);
	if ( msecs != FOREVER )
		mt_enqueue_time(mt_curr_task, msecs_to_ticks(msecs));
	scheduler();
	success = mt_curr_task->success;
	RestoreInts();

	return success;
}

/*
--------------------------------------------------------------------------------
SignalQueue, FlushQueue - funciones para despertar procesos en una cola

SignalQueue despierta el ultimo proceso de la cola (el de mayor prioridad o
el que llego primero entre dos de la misma prioridad), el valor de retorno 
es true si desperto a un proceso. Este proceso completa su WaitQueue() 
exitosamente.
FlushQueue despierta a todos los procesos de la cola, que completan su
WaitQueue() con el resultado que se pasa como argumento.
--------------------------------------------------------------------------------
*/

bool		
SignalQueue(TaskQueue_t *queue)
{
	Task_t *task;

	DisableInts();
	if ( task = mt_getlast(queue) )
	{
		ready(task, true);
		scheduler();
	}
	RestoreInts();

	return task != NULL;
}

void			
FlushQueue(TaskQueue_t *queue, bool success)
{
	Task_t *task;

	DisableInts();
	if ( mt_peeklast(queue) )
	{
		while ( task = mt_getlast(queue) )
			ready(task, success);
		scheduler();
	}
	RestoreInts();
}

/*
--------------------------------------------------------------------------------
Send, SendCond, SendTimed - enviar un mensaje
--------------------------------------------------------------------------------
*/

bool			
Send(Task_t *to, void *msg, unsigned size)
{
	return SendTimed(to, msg, size, FOREVER);
}

bool			
SendCond(Task_t *to, void *msg, unsigned size)
{
	return SendTimed(to, msg, size, 0);
}

bool			
SendTimed(Task_t *to, void *msg, unsigned size, Time_t msecs)
{
	bool success;

	DisableInts();

	if ( to->state == TaskReceiving && (!to->from || to->from == mt_curr_task) )
	{
		to->from = mt_curr_task;
		if ( to->msg && msg )
		{
			if ( size > to->size )
				Panic("Buffer insuficiente para transmitir mensaje");
			to->size = size;
			memcpy(to->msg, msg, size);
		}
		else
			to->size = 0;
		ready(to, true);
		scheduler(); /*cada vez que despierto a alguien por si acaso llamo al scheduler*/
		RestoreInts();
		return true;
	}

	if ( !msecs )
	{
		RestoreInts();
		return false;
	}

	mt_curr_task->msg = msg;
	mt_curr_task->size = size;
	mt_curr_task->state = TaskSending;
	mt_enqueue(mt_curr_task, &to->send_queue);
	if ( msecs != FOREVER )
		mt_enqueue_time(mt_curr_task, msecs_to_ticks(msecs));
	scheduler();/*esta llamada me quila la cpu*/
	success = mt_curr_task->success;

	RestoreInts();
	return success;
}


/*
--------------------------------------------------------------------------------
Receive, ReceiveCond, ReceiveTimed - recibir un mensaje
--------------------------------------------------------------------------------
*/

bool			
Receive(Task_t **from, void *msg, unsigned *size)
{
	return ReceiveTimed(from, msg, size, FOREVER);
}

bool			
ReceiveCond(Task_t **from, void *msg, unsigned *size)
{
	return ReceiveTimed(from, msg, size, 0);
}

bool			
ReceiveTimed(Task_t **from, void *msg, unsigned *size, Time_t msecs)
{
	bool success;
	Task_t *sender;

	DisableInts();

	if ( from && *from )/*recepci—n selectiva de un proceso en particular*/
		sender = (*from)->queue == &mt_curr_task->send_queue ? *from : NULL;/*me fijo si el est‡ bloquado en la cola mia, sino
																			 no tengo emisor de mensaje, no hay proceso
																			 bloqueado enviandome mensaje*/
	else
		sender = mt_peeklast(&mt_curr_task->send_queue);/*se fija el œltimo que es el de mayor prioridad*/

	if ( sender ) /*si tengo alguno que me esta enviando un mensaje , lo recibo*/
	{
		if ( from ) 
			*from = sender;
		if ( sender->msg && msg )
		{
			if ( size )
			{
				if ( sender->size > *size )
					Panic("Buffer insuficiente para recibir mensaje");
				memcpy(msg, sender->msg, *size = sender->size);/*en size dejo el verdadero tam del msg*/
			}
		}
		else if ( size )
			*size = 0;
		ready(sender, true);
		scheduler();
		RestoreInts();
		return true;
	}

	if ( !msecs )/*si el timeout el cero, fracaso*/
	{
		RestoreInts();
		return false;
	}
	
	/*si no tengo nadie que me envie me tengo que bloquear recibiendo*/
	mt_curr_task->from = from ? *from : NULL;
	mt_curr_task->msg = msg;
	mt_curr_task->size = size ? *size : 0;
	mt_curr_task->state = TaskReceiving;
	if ( msecs != FOREVER )
		mt_enqueue_time(mt_curr_task, msecs_to_ticks(msecs));/*me bloquedo*/
	scheduler();
	if ( success = mt_curr_task->success )
	{
		if ( size )
			*size = mt_curr_task->size;
		if ( from )
			*from = mt_curr_task->from;
	}

	RestoreInts();
	return success;
}

/*
--------------------------------------------------------------------------------
mt_select_task - determina el proximo proceso a ejecutar.

Retorna true si ha cambiado el proceso en ejecucion.
Llamada desde scheduler() y cuanto retorna una interrupcion de primer nivel.
Guarda y restaura el contexto del coprocesador y el contexto propio del usuario,
si existe. 
--------------------------------------------------------------------------------
*/

bool 
mt_select_task(void)/*ES LA LOGICA DEL SCHEDULER*/
{
	Task_t *ready_task;

	/* Ver si el proceso actual puede conservar la CPU */
	if ( mt_curr_task->state == TaskCurrent )
	{
		if ( mt_curr_task->atomic_level )		/* No molestar */
			return false;

		/* Analizar prioridades y ranura de tiempo */
		ready_task = mt_peeklast(&ready_q); /*candidato ultimo de la colad de procesos listos para ejecutar*/
		if ( !ready_task || ready_task->priority < mt_curr_task->priority ||
				ticks_to_run && ready_task->priority == mt_curr_task->priority )
			return false; 

		/* El proceso actual pierde la CPU */
		ready(mt_curr_task, false); /*lo enviamos a la cola de procesos listos para ejecutar*/
	}

	/* Obtener el proximo proceso */
	last_task = mt_curr_task;
	mt_curr_task = mt_getlast(&ready_q);
	mt_curr_task->state = TaskCurrent;

	/* Si es el mismo de antes, no hay nada mas que hacer */
	if ( mt_curr_task == last_task )
		return false;

#ifdef HIREGS
	/* Guardar/reponer registros superiores si existen */
	if ( use_hiregs )
	{
		save_hiregs(last_task);/*se guardan los registros de la tarea anterior*/
		restore_hiregs(mt_curr_task);/*se recuperan los de la actual*/
	}
#endif
	
	/* Guardar/reponer contexto aritmetico si corresponde */
	if ( last_task->math_data )
		mt_save_math(last_task);
	if ( mt_curr_task->math_data )
		mt_restore_math(mt_curr_task);

	/* Guardar/reponer contexto propio del usuario */
	if ( save_restore )
		save_restore(last_task, mt_curr_task);

	/* Inicializar ranura de tiempo */
	ticks_to_run = QUANTUM;
	return true;
}

/*
--------------------------------------------------------------------------------
scheduler - selecciona el proximo proceso a ejecutar.

Se llama cuando se bloquea el proceso actual o se despierta cualquier proceso.
No hace nada si se llama desde una interrupcion, porque las interrupciones
pueden despertar procesos pero recien se cambia contexto al retornar de la
interrupcion de primer nivel.
--------------------------------------------------------------------------------
*/

static void
scheduler(void)
{
	if ( !mt_int_level && mt_select_task() )
		context_switch();/*cambio de contexto principal, cambio el stack frame*/
}

/*
--------------------------------------------------------------------------------
context_switch - cambio de contexto

Por ser una función de tipo interrupt, realiza el guardado y recuperacion del
contexto de registros en el stack. Llamada desde scheduler().
--------------------------------------------------------------------------------
*/

static void interrupt
context_switch(void)
{
	last_task->ss = _SS;/*registro ss*/
	last_task->sp = _SP;/*resgistro sp*/
	_SS = mt_curr_task->ss;
	_SP = mt_curr_task->sp;
}

/*
--------------------------------------------------------------------------------
clockint - interrupcion de tiempo real

Despierta a los procesos de la cola de tiempo que tengan su cuenta de ticks
agotada, y decrementa la cuenta del primero que quede en la cola.
Decrementa la ranura de tiempo del proceso actual.
--------------------------------------------------------------------------------
*/

static void 
clockint(unsigned irq)
{
	Task_t *task;

	OldInterrupt(irq);
	disable();

	if ( ticks_to_run )
		ticks_to_run--;
	while ( (task = mt_peekfirst_time()) && !task->ticks )
	{
		mt_getfirst_time();
		ready(task, false);
	}
	if ( task )
		task->ticks--;
}

/*
--------------------------------------------------------------------------------
Atomic - deshabilita el modo preemptivo para el proceso actual (anidable)
--------------------------------------------------------------------------------
*/

void
Atomic(void)
{
	++mt_curr_task->atomic_level;
}

/*
--------------------------------------------------------------------------------
Unatomic - habilita el modo preemptivo para el proceso actual (anidable)
--------------------------------------------------------------------------------
*/

void
Unatomic(void)
{
	if ( mt_curr_task->atomic_level && !--mt_curr_task->atomic_level )
	{
		DisableInts();
		scheduler();
		RestoreInts();
	}
}

/*
--------------------------------------------------------------------------------
do_nothing - Proceso nulo

Corre con prioridad 0 y toma la CPU cuando ningun otro proceso puede ejecutar.
--------------------------------------------------------------------------------
*/

#pragma argsused
static void
do_nothing(void *arg)
{
	while ( true )
		;
}

#ifdef HIREGS

/*
--------------------------------------------------------------------------------
check_hiregs - determina si existen los registros de 32 bits (eax, ebx, etc.)

Averigua si el procesador es un 80386+. Para ello intenta resetear el bit 15 y
setear los bits 12-14 de la palabra de flags. En el 8086 los bits 12-15 son
siempre 1, y en el 80286 los bits 12-14 son siempre 0.
--------------------------------------------------------------------------------
*/

static void
check_hiregs(void)
{
	unsigned flags;

	asm {
		pushf
		pop		flags
	}
	flags &= 0x7fff;
	flags |= 0x7000;
	asm {
		push	flags
		popf
		pushf
		pop		flags
	}
	use_hiregs = (flags & 0xf000) == 0x7000;
}

/*
--------------------------------------------------------------------------------
save_hiregs, restore_hiregs - guardar/reponer registros superiores.

Guardan y reponen la parte superior de los registros de 32 bits (eax, ebx, etc.)
--------------------------------------------------------------------------------
*/

static void
save_hiregs(Task_t *task)
{
	HiRegs_t *hr = &task->hi_regs;

	hr->hi_eax = mt_gethi_eax();
	hr->hi_ebx = mt_gethi_ebx();
	hr->hi_ecx = mt_gethi_ecx();
	hr->hi_edx = mt_gethi_edx();
	hr->hi_esi = mt_gethi_esi();
	hr->hi_edi = mt_gethi_edi();
}

static void restore_hiregs(Task_t *task)
{
	HiRegs_t *hr = &task->hi_regs;

	mt_sethi_eax(hr->hi_eax);
	mt_sethi_ebx(hr->hi_ebx);
	mt_sethi_ecx(hr->hi_ecx);
	mt_sethi_edx(hr->hi_edx);
	mt_sethi_esi(hr->hi_esi);
	mt_sethi_edi(hr->hi_edi);
}

#endif

/*
--------------------------------------------------------------------------------
init_kernel, end_kernel - inicializacion y cleanup del kernel

Init_kernel crea el proceso nulo y el principal, captura las interrupciones y 
establece el manejador de la interrupcion de tiempo real.
El proceso principal funciona como cualquier otro proceso y se le pueden
aplicar las mismas operaciones. La unica diferencia es que no tiene un stack
propio (utiliza el stack inicial del programa). Este proceso no se crea con 
CreateTask() y tampoco puede ser dado de baja con DeleteTask(); cuando retorna 
se termina el programa.
--------------------------------------------------------------------------------
*/

static void
init_kernel(void)
{
	directvideo = true;

#ifdef HIREGS
	/* determinar presencia de registros de 32 bits */
	check_hiregs();
#endif

	/* capturar control-break */
	signal(SIGINT, exit);

	/* inicializar proceso principal */
	main_task.name = "Main Task";
	main_task.state = TaskCurrent;
	main_task.priority = DEFAULT_PRIO;
	main_task.send_queue.name = main_task.name;
	ticks_to_run = QUANTUM;
	mt_curr_task = &main_task;

	/* crear proceso nulo y ponerlo ready */
	ready(CreateTask(do_nothing, NULL_STACK, NULL, "Null Task", MIN_PRIO), 
			false);

	/* inicializar modulo aritmetico */
	mt_setup_math();

	/* capturar interrupciones y establecer la de tiempo real */
	mt_setup_irqs();
	SetHandler(CLOCKIRQ, clockint);
}

#pragma startup init_kernel /*se ejecuta antes del main. en gcc __attribute__((constructor)) o destructor*/

static void
end_kernel(void)
{
	Atomic();
	mt_restore_irqs();
}

#pragma exit end_kernel
