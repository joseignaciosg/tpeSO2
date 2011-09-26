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
extern int CurrentPID;
int FirstTime=1;
extern PROCESS idle;
extern PROCESS procesos[];

int timeslot = TIMESLOT;


void SaveESP (int);
void* GetTemporaryESP (void);
PROCESS* GetNextProcess (void);
PROCESS* GetNextTask(void);
int LoadESP(PROCESS*);
int NoProcesses(void);
int isTimeSlot(void);


char idleprocess[0x400];

int isTimeSlot()
{
	return --timeslot;
}

void SaveESP (int ESP)
{
	timeslot = TIMESLOT;
	PROCESS* temp;
	if (!FirstTime)
	{
		temp=GetProcessByPID (CurrentPID);
		temp->ESP=ESP;
	}
	FirstTime=0;
	return;
}

void* GetTemporaryESP (void)
{
	return (void*)idle.ESP;
}

PROCESS* GetNextProcess (void)
{
	char* video=(char*)0xb8000;
	PROCESS* temp;
	//selecciona la tarea
	temp=GetNextTask();
	temp->lastCalled=0;
	CurrentPID=temp->pid;
	last100[counter100]=CurrentPID;
	counter100=(counter100+1)%100;
	return temp;
	
}

PROCESS* GetNextTask()
{
	int i;
	int winner;
	int winnervalue=-1;

	if (NoProcesses())
		return &idle;
	for(i=0;i<64;i++)
	{
		if(!procesos[i].free)
		{
			if(procesos[i].sleep>=0)
			{
				if(procesos[i].blocked == 0)
				{
					procesos[i].lastCalled++;
					int temp;
					temp=(MAX_PRIORITY-procesos[i].priority)*PRIORITY_RATIO+procesos[i].lastCalled;
					if (temp>winnervalue)
					{
						winnervalue=temp;
						winner=i;
					}
				}
			}
			else
				procesos[i].sleep++;
		}
	}
	if (winnervalue>-1)
		return &procesos[winner];
	else
		return &idle;
}

int LoadESP(PROCESS* proc)
{
	return proc->ESP;
}

void SetupScheduler(void)
{
	int i;

	for (i=0;i<64;i++)
		procesos[i].free=1;
	
	/*idleprocess= Malloc(0x200);*/
	idle.pid = 0;
	idle.foreground = 0;
	idle.priority = 4;
	/*memcpy2(idle.name,"Idle",5);*/
	idle.sleep = 0;
	idle.blocked = 0;
	idle.tty = 0;
	idle.stackstart = (int)(&idleprocess);
	idle.stacksize = 0x200;
	procesos[i].parent = 0;
	idle.lastCalled = 0;
	idle.ESP=LoadStackFrame(Idle,0,0,(int)(idleprocess+0x1FF),Cleaner);
	
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
	for (i=0;i<64;i++)
	{
		if (procesos[i].free==0)
			return 0;
	}
	return 1;
}
