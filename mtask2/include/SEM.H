#ifndef SEM_H
#define SEM_H

#include "mtask.h"

typedef struct Semaphore_t
{
	unsigned		value;
	TaskQueue_t *	queue;
}
Semaphore_t;

Semaphore_t *	CreateSem(char *name, unsigned value);
void 			DeleteSem(Semaphore_t *sem);
bool 			WaitSem(Semaphore_t *sem);
bool 			WaitSemCond(Semaphore_t *sem);
bool 			WaitSemTimed(Semaphore_t *sem, Time_t msecs);
void 			SignalSem(Semaphore_t *sem);
unsigned		ValueSem(Semaphore_t *sem);
void 			FlushSem(Semaphore_t *sem, bool wait_ok);

#endif
