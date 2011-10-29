#ifndef MTASK_H
#define MTASK_H

#define HIREGS

#define MIN_PRIO		0
#define DEFAULT_PRIO	50
#define MAX_PRIO		(-1U)
#define FOREVER			(-1UL)

typedef unsigned long Time_t;
typedef unsigned Prio_t;

typedef enum
{ 
	false, 
	true 
} 
bool;

typedef enum 
{ 
	TaskSuspended, 
	TaskReady, 
	TaskCurrent, 
	TaskDelaying, 
	TaskWaiting, 
	TaskSending, 
	TaskReceiving, 
	TaskTerminated 
} 
TaskState_t;

typedef struct Task_t Task_t;

typedef struct
{
	char *			name;
	Task_t *		head;
	Task_t *		tail;
}
TaskQueue_t;

#ifdef HIREGS

typedef struct
{
	unsigned		hi_eax;
	unsigned		hi_ebx;
	unsigned		hi_ecx;
	unsigned		hi_edx;
	unsigned		hi_esi;
	unsigned		hi_edi;
}
HiRegs_t;

#endif

struct Task_t
{
	char *			name;
	TaskState_t		state;
	Prio_t			priority;
	unsigned		atomic_level;/*para decir si es preempivo o no, es distinto que bloquear interrupciones*/
	char *			stack;
	unsigned short	ss;
	unsigned short	sp;
	void *			math_data;
	TaskQueue_t	*	queue;/*cola de tiempo?*/
	Task_t *		prev;
	Task_t *		next;
	bool			success;
	bool			in_time_q;
	Task_t *		time_prev; /*dos links al sig y anterior en la cola de procesos*/
	Task_t *		time_next;
	Time_t			ticks; /*que faltan para despertarlo*/
	void *			data;
	Task_t *		from;
	void *			msg;/*se usan para enviar o recibir mensajes*/
	unsigned 		size;
	TaskQueue_t 	send_queue;/*cola de los procesos bloqueados que tratan de mandarle un mensaje este*/
#ifdef HIREGS
	HiRegs_t		hi_regs;
#endif
};

typedef void (*TaskFunc_t)(void *arg);
typedef void (*Handler_t)(unsigned irq); /*manejador de interrupciones*/
typedef void (*Switcher_t)(Task_t *save, Task_t *restore);

/* API */

Task_t *		CreateTask(TaskFunc_t func, unsigned stacksize, void *arg, /*para crear un proceso, se crea suspendido*/
									char *name, Prio_t priority);
Task_t *		CurrentTask(void);
void			DeleteTask(Task_t *task);

void			ProtectMath(Task_t *task);/*para proteger los registros del coprocesador*/
Prio_t			GetPriority(Task_t *task);
void			SetPriority(Task_t *task, Prio_t priority);
void			Suspend(Task_t *task);
void			Ready(Task_t *task);/*para ponerla lista para ejecutar*/

TaskQueue_t *	CreateQueue(char *name);
void			DeleteQueue(TaskQueue_t *queue);
bool			WaitQueue(TaskQueue_t *queue);/*bloquerase en una cola de procesos*/
bool			WaitQueueTimed(TaskQueue_t *queue, Time_t msecs);/*bloquearse en un timeout*/
bool			SignalQueue(TaskQueue_t *queue);/*para despertar a todos los de la colita*/
void			FlushQueue(TaskQueue_t *queue, bool success);/*vaciar la cola, en success se le pasa como se quiere que se 
															terminen todos los procesos de las colas*/

bool			Send(Task_t *to, void *msg, unsigned size);/*enviar mensaje, bloqueante*/
bool			SendCond(Task_t *to, void *msg, unsigned size);/*si no se levanta el mensaje, retorna false al toque*/
bool			SendTimed(Task_t *to, void *msg, unsigned size, Time_t msecs);
bool			Receive(Task_t **from, void *msg, unsigned *size);/*le pasas null en from, si quiero recibir de cualquiera, 
																   le paso la direecion de un puntero inicializado en null 
																   recibo de cualquiera pero se cual es, le paso una variable
																   y recibo solamente de ese*/
bool			ReceiveCond(Task_t **from, void *msg, unsigned *size);
bool			ReceiveTimed(Task_t **from, void *msg, unsigned *size,
									Time_t msecs);

void			Pause(void);/*bloquearse si esperar nada, se despierta con ready*/
void			Yield(void);/*cede la cpu voluntariamente, si hay un proceso de mayor prioridad esperando, o si
							 hay uno esperando de mi misma prioridad y ya se me acabó la ranura de tiempo*/
void			Delay(Time_t msecs);
void			Exit(void);

void			Atomic(void);/*para incrementar y decrementar la variable atomic para determinar si se esta funcionando
							 peemtivamente o no*/
void			Unatomic(void);

void			SetHandler(unsigned irq, Handler_t handler);
void			OldInterrupt(unsigned irq);/*para llamar a la interrupcion anterior*/
void			EndInterrupt(unsigned irq);/*manda el EOI al pic*/
void 			DisableIRQ(unsigned irq);/*habilita o deshabilidar en el IMR*/
void 			EnableIRQ(unsigned irq);

void *			Malloc(unsigned size);/*malloc de c serializados en el modo atómico, no se espera que una interrupcion 
									   aloque memoria*/
void *			Realloc(void *mem, unsigned size);
char *			StrDup(char *str);
void 			Free(void *mem);

void			SetData(Task_t *task, void *data);
void			SetSwitcher(Switcher_t switcher);

void			Panic(char *msg);

#define			DisableInts()	asm { pushf; cli }/*macros para activar y desactivar interrupciones*/
#define			RestoreInts()	asm { popf }

#endif
