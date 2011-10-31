/********************************** 
*
*  scheduler2.c
*  	Galindo, Jose Ignacio
*  	Homovc, Federico
*  	Loreti, Nicolas
*		ITBA 2011
*
***********************************/

#include "../include/defs.h"
#include "../include/kernel.h"
#include "../include/utils.h"
#include "../include/stdio.h"

int last100[100]={0};
int counter100 = 0;
int FirstTime = 1;
int actualKilled = 0;
int timeslot = TIMESLOT;

extern int CurrentPID;
extern int currentProcessTTY;
extern int currentTTY;
extern PROCESS idle;
extern processList ready;

void SaveESP (int);
PROCESS * GetNextProcess (void);
PROCESS * GetNextTask(void);
int LoadESP(PROCESS*);
int isTimeSlot(void);


int isTimeSlot()
{
	processNode * node;
	
	node = ((processNode*)ready);
	while(node != NULL)
	{
		if(node->process->sleep)
		{
			node->process->sleep--;
			if(!node->process->sleep)
				awake_process(node->process->pid);
		}
		node = ((processNode*)node->next);
	}
	if(--timeslot)
		return timeslot;
	timeslot = TIMESLOT;
	return 0;
}

void SaveESP (int ESP)
{
	timeslot = TIMESLOT;
	PROCESS* temp;
	if (!FirstTime && !actualKilled)
	{
		temp = (PROCESS *)GetProcessByPID(CurrentPID);
		temp->ESP = ESP;
		if(temp->state == RUNNING)
			temp->state = READY;
	}
	FirstTime = actualKilled = 0;
	return;
}

PROCESS * GetNextProcess(void)
{
	PROCESS * temp;
	temp = GetNextTask();
	temp->state = RUNNING;
	CurrentPID = temp->pid;
	if(temp->pid == 0)
		currentProcessTTY = currentTTY;
	else
		currentProcessTTY = temp->tty;
	last100[counter100] = CurrentPID;
	counter100 = (counter100 + 1) % 100;
	return temp;
}

PROCESS * GetNextTask()
{
	processNode * aux;
	int maxAcum = 0, maxPriority = -1;
	PROCESS * proc = NULL;

	if(ready == NULL)
		return &idle;

	aux = ((processNode *)ready);

	while(aux != NULL)
	{
		if(aux->process->acum >= maxAcum && aux->process->state == READY)
		{
			if(aux->process->acum == maxAcum)
			{
				if(aux->process->priority >= maxPriority)
				{
					maxPriority = aux->process->priority;
					proc = aux->process;
					maxAcum = aux->process->acum;
				}
			}
			else
			{
				maxPriority = aux->process->priority;
				proc = aux->process;
				maxAcum = aux->process->acum;
			}
		}
		if(aux->process->state == READY)
			aux->process->acum += aux->process->priority + 1;
		aux = ((processNode*)aux->next);
	}

	if(!proc)
		return &idle;
	
	proc->acum = 0;
	return proc;
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
	idle.name = (char*)malloc(8);
	memcpy(idle.name, "Idle", str_len("Idle") + 1);
	idle.state = READY;
	idle.tty = 0;
	idle.stackstart = (int)idleprocess;
	idle.stacksize = 0x200;
	idle.parent = -1;
	idle.waitingPid = -1;
	idle.ESP = LoadStackFrame(Idle,0,0,(int)(idleprocess + 0x1FF), end_process);
	
	return;
}
