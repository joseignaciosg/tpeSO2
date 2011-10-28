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
	//printf("timeslot:%d", timeslot);
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
		temp = GetProcessByPID(CurrentPID);
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
	idle.name = (char*)malloc(10);
	memcpy(idle.name, "Idle", str_len("Idle") + 1);
	idle.state = READY;
	idle.tty = 0;
	idle.stackstart = (int)idleprocess;
	idle.stacksize = 0x200;
	idle.parent = -1;
	idle.waitingPid = -1;
	idle.ESP = LoadStackFrame(Idle,0,0,(int)(idleprocess+0x1FF), end_process);
	
	return;
}
