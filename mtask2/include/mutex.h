#ifndef MUTEX_H
#define MUTEX_H

#include "sem.h"
#include "mtask.h"

typedef struct Mutex_t
{
	unsigned		use_count;
	Task_t *		owner;
	Semaphore_t *	sem;
}
Mutex_t;

Mutex_t *		CreateMutex(char *name);
void 			DeleteMutex(Mutex_t *mut);
bool			EnterMutex(Mutex_t *mut);
bool			EnterMutexCond(Mutex_t *mut);
bool			EnterMutexTimed(Mutex_t *mut, Time_t msecs);
void			LeaveMutex(Mutex_t *mut);

#endif
