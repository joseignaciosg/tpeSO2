#ifndef MONITOR_H
#define MONITOR_H

#include "sem.h"
#include "mtask.h"

typedef struct
{
	Task_t *		owner;
	Semaphore_t *	sem;
}
Monitor_t;

Monitor_t *			CreateMonitor(char *name);
void 				DeleteMonitor(Monitor_t *mon);
bool				EnterMonitor(Monitor_t *mon);
bool				EnterMonitorCond(Monitor_t *mon);
bool				EnterMonitorTimed(Monitor_t *mon, Time_t msecs);
void				LeaveMonitor(Monitor_t *mon);

typedef struct
{
	Monitor_t *		monitor;
	TaskQueue_t *	queue;
}
Condition_t;

Condition_t *		CreateCondition(char *name, Monitor_t *mon);
void				DeleteCondition(Condition_t *cond);
bool				WaitCondition(Condition_t *cond);
bool				WaitConditionTimed(Condition_t *cond, Time_t msecs);
bool				SignalCondition(Condition_t *cond);
void				BroadcastCondition(Condition_t *cond);

#endif
