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
extern PROCESS procesos[PROCESS_QTTY];
extern processList ready;
extern int roundRobin;

int timeslot = TIMESLOT;

void SaveESP (int);
PROCESS * GetNextProcess (void);
PROCESS * GetNextTask(void);
int LoadESP(PROCESS*);
int NoProcesses(void);
int isTimeSlot(void);
void* GetTemporaryESP (void);

char idleprocess[0x400];



void* GetTemporaryESP (void)
{
	return (void*)idle.ESP;
}

int isTimeSlot()
{
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
	}
	FirstTime = 0;
	return;
}

PROCESS * GetNextProcess(void)
{
	PROCESS * temp;
	temp = GetNextTask();
	CurrentPID = temp->pid;
	last100[counter100] = CurrentPID;
	counter100 = (counter100 + 1) % 100;
	return temp;
}

PROCESS * GetNextTask()
{
	processNode * aux;

	if(ready == NULL)
		return &idle;
	
	if(CurrentPID == 0)
		return ((processNode*)ready)->process;

	aux = ((processNode *)ready);
	while(aux->process->pid != CurrentPID)
		aux = ((processNode*)aux->next);

	if(aux->next == NULL)
		return ((processNode*)ready)->process;
		
	return ((processNode*)aux->next)->process;
}

int LoadESP(PROCESS* proc)
{
	return proc->ESP;
}

void SetupScheduler(void)
{
	int i;

	for (i = 0; i < PROCESS_QTTY; i++)
		procesos[i].free = 1;
	
	idle.pid = 0;
	idle.foreground = 0;
	idle.priority = 4;
	memcpy(idle.name,"Idle",5);
	idle.sleep = 0;
	idle.blocked = 0;
	idle.tty = 0;
	idle.stackstart = (int)(&idleprocess);
	idle.stacksize = 0x400;
	procesos[i].parent = 0;
	idle.lastCalled = 0;
	idle.ESP = LoadStackFrame(Idle,0,0,(int)(&idleprocess + 0x3FF),Cleaner);
	
	return;
}

void Cleaner(void)
{
	char Men[10];
	_Cli();
	Destroy(CurrentPID);
	k_clear_screen();
	/*mess("==>");*/
	_Sti();
	while(1);
	
}

int NoProcesses()
{
	int i;
	for (i = 0; i < PROCESS_QTTY; i++)
	{
		if (procesos[i].free == 0)
			return 0;
	}
	return 1;
}
