/********************************** 
*
*  scheduler.c
*  	Galindo, Jose Ignacio
*  	Homovc, Federico
*		ITBA 2011
*
***********************************/

#include "../include/defs.h"
#include "../include/kc.h"

int last100[100]={0};
int counter100;
int FirstTime = 1;
extern int CurrentPID;
extern PROCESS idle;
extern processList ready;
extern int roundRobin;

int timeslot = TIMESLOT;

void SaveESP (int);
PROCESS * GetNextProcess (void);
PROCESS * GetNextTask(void);
int LoadESP(PROCESS*);
int isTimeSlot(void);
void* GetTemporaryESP (void);


void* GetTemporaryESP (void)
{
	return (void*)idle.ESP;
}

int isTimeSlot()
{
	//printf("timeslot:%d", timeslot);
	if(--timeslot)
		return timeslot;
	timeslot = TIMESLOT;
	return 0;
}

void SaveESP (int ESP)
{
	timeslot = TIMESLOT;
	PROCESS* temp;
	if (!FirstTime)
	{
		temp = GetProcessByPID(CurrentPID);
		temp->ESP = ESP;
		if(temp->state == RUNNING)
			temp->state = READY;
	}
	FirstTime = 0;
	return;
}

PROCESS * GetNextProcess(void)
{
	PROCESS * temp;
	temp = GetNextTask();
	temp->state = RUNNING;
	CurrentPID = temp->pid;
	//printf("PID: %d\n", CurrentPID);
	//printf("process name: %s\n", temp->name);
	last100[counter100] = CurrentPID;
	counter100 = (counter100 + 1) % 100;
	return temp;
}

PROCESS * GetNextTask()
{
	processNode * aux;
	int flag = 0, beginning = 0;

	if(ready == NULL)
		return &idle;

	aux = ((processNode *)ready);

	if(CurrentPID == 0)
	{
		while(aux != NULL)
		{
			if(aux->process->state == READY)
				return aux->process;
			aux = ((processNode*)aux->next);
		}
		return &idle;
	}

	while(aux != NULL)
	{
		if(aux->process->pid == CurrentPID)
			flag = 1;
		aux = ((processNode*)aux->next);
		if(flag && aux != NULL && aux->process->state == READY)
			return aux->process;
		if(aux == NULL && !beginning)
		{
			beginning = 1;
			aux = ((processNode*)ready);
			if(aux->process->state == READY)
				return aux->process;
		}
	}

	return &idle;

}

int LoadESP(PROCESS* proc)
{
	return proc->ESP;
}

void SetupScheduler(void)
{
	void * idleprocess;

	idleprocess = (void *)malloc(0x200);
	idle.pid = 0;
	idle.foreground = 0;
	idle.priority = 4;
	memcpy(idle.name, "Idle", str_len("Idle") + 1);
	idle.state = READY;
	idle.tty = 0;
	idle.stackstart = (int)idleprocess;
	idle.stacksize = 0x200;
	idle.parent = 0;
	idle.ESP = LoadStackFrame(Idle,0,0,(int)(idleprocess+0x1FF), end_process);
	
	return;
}
