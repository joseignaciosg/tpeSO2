/********************************** 
*
*  kernel.c
*  	Galindo, Jose Ignacio
*  	Homovc, Federico
*		ITBA 2011
*
***********************************/


/***	Project Includes	***/
#include "../include/kasm.h"
#include "../include/kernel.h"
#include "../include/kc.h"
#include "../include/shell.h"
#include "../include/utils.h"

DESCR_INT idt[0x90]; /* IDT 144 positions*/
IDTR idtr;			 /* IDTR */

PROCESS idle;
processList ready;
static int nextPID = 1;
int CurrentPID = 0;
TTY terminals[4];
int currentTTY = 0;
int roundRobin = 0;
user admin;
extern int timeslot;

PROCESS procesos[5];
STACK_FRAME stacks[5];

void startTerminal(int pos);
void set_Process_ready(PROCESS * proc);
void block_process(int pid);
void awake_process(int pid);
void Teta(int argc, char* argv[]);
void Teta1(int argc, char* argv[]);
void * malloc (int size);
void kill(int pid);

void
initializeIDT()
{
	setup_IDT_entry (&idt[0x08], 0x08, (dword)&_int_08_hand, ACS_INT, 0);
	setup_IDT_entry (&idt[0x09], 0x08, (dword)&_int_09_hand, ACS_INT, 0);
	setup_IDT_entry (&idt[0x80], 0x08, (dword)&_int_80_hand, ACS_INT, 0);
	idtr.base = 0;
	idtr.base +=(dword) &idt;
	idtr.limit = sizeof(idt)-1;
	_lidt (&idtr);
}


void
unmaskPICS(){
	_mascaraPIC1(0xFC);
   	_mascaraPIC2(0xFF);
}


void
setup_IDT_entry (DESCR_INT *item, byte selector, dword offset, byte access,
			 byte cero) {
  item->selector = selector;
  item->offset_l = offset & 0xFFFF;
  item->offset_h = offset >> 16;
  item->access = access;
  item->cero = cero;
  
  return;
}


void
reboot(){
	_export( 0x64, 0xFE); /* pulse CPU reset line */
	return;
}

/**********************************************
Starting point of the whole OS
*************************************************/
int
kmain()
{
	int i;
	_Cli();
	k_clear_screen();

	initializeIDT();
	unmaskPICS();
	SetupScheduler();
	ready = NULL;
	for(i = 0; i < 4; i++)
		startTerminal(i);
	CreateProcessAt("Shell0", (int(*)(int, char**))shell, 0, 0, (char**)0, 0x400, 2, 1);
	CreateProcessAt("Shell1", (int(*)(int, char**))shell, 1, 1, (char**)0, 0x400, 2, 1);
	CreateProcessAt("Shell2", (int(*)(int, char**))shell, 2, 2, (char**)0, 0x400, 2, 1);
	CreateProcessAt("Shell3", (int(*)(int, char**))shell, 3, 3, (char**)0, 0x400, 2, 1);
	strcopy(admin.name, "chinux", str_len("chinux"));
	strcopy(admin.password, "chinux", str_len("chinux"));
	_Sti();
	while(TRUE)
	;
	return 1;
}

int CreateProcessAt(char* name, int (*process)(int,char**),int tty, int argc, char** argv, int stacklength, int priority, int isFront)
{
	PROCESS * proc;
	void * stack = malloc(stacklength);
	proc = malloc(sizeof(PROCESS));
	proc->name = (char*)malloc(10);
	proc->pid = nextPID++;
	proc->foreground = isFront;
	proc->priority = priority;
	memcpy(proc->name,name,str_len(name) + 1);
	proc->state = READY;
	proc->tty = tty;
	proc->stacksize = stacklength;
	proc->stackstart = (int)stack;
	proc->ESP = LoadStackFrame(process,argc,argv,(int)(stack + stacklength - 1), end_process);
	proc->parent = CurrentPID;
	proc->waitingPid = 0;
	set_Process_ready(proc);	
	return proc->pid;
	
}

int LoadStackFrame(int(*process)(int,char**),int argc,char** argv, int bottom, void(*cleaner)())
{
	STACK_FRAME * frame = (STACK_FRAME*)(bottom - sizeof(STACK_FRAME));
	frame->EBP = 0;
	frame->EIP = (int)process;
	frame->CS = 0x08;
	
	frame->EFLAGS = 0;
	frame->retaddr = cleaner;
	frame->argc = argc;
	frame->argv = argv;
	return (int)frame;
}

void set_Process_ready(PROCESS * proc)
{
	processNode * node;
	processNode * aux;
	node = malloc(sizeof(processNode));
	aux = malloc(sizeof(processNode));
	node->next = NULL;
	node->process = proc;
	if(ready == NULL)
	{
		ready = (processList)node;
		return;
	}

	aux = ((processNode*)ready);
	while(aux->next != NULL)
		aux = (processNode*)aux->next;
	aux->next = (processList)node;
	return;
}

void block_process(int pid)
{
	processNode * aux;
	processNode * proc;

	timeslot = 1;					/*"yield"*/
	if(ready == NULL)
	{
		_yield();
		return;
	}
	

	printf("blocking process: %d\n", pid);
	aux = ((processNode*)ready);
	if(aux->process->pid == pid)
	{
		aux->process->state = BLOCKED;
	}else{
		while(aux->next != NULL && ((processNode*)aux->next)->process->pid != pid)
			aux = ((processNode*)aux->next);
		if(aux->next == NULL)
		{
			printf("error fatal!!\n");
			_yield();
			return;
		}
		((processNode*)aux->next)->process->state = BLOCKED;
	}
	_yield();
	return;
}

/*awakes all processes from the given tty that are blocked*/
void awake_process(int pid)
{
	processNode * proc;

	printf("awakening process: %d\n", pid);
	proc = ((processNode*)ready);
	while(proc != NULL)
	{
		if(proc->process->pid == pid && proc->process->state == BLOCKED && !proc->process->waitingPid)
			proc->process->state = READY;
		proc = ((processNode*)proc->next);
	}
	return ;
}


PROCESS * GetProcessByPID (int pid)
{
	processNode * aux;
	if(pid == 0 || ready == NULL)
		return &idle;
	aux = ((processNode*)ready);
	while(aux != NULL && aux->process->pid != pid)
		aux = ((processNode*)aux->next);
	if(aux == NULL)
		return &idle;
	return aux->process;
}

int Idle(int argc, char* argv[])
{
	_Sti();
	while(1)
	;
}

void end_process(void)
{
	PROCESS * proc;
	PROCESS * parent;
	
	_Cli();

	proc = GetProcessByPID(CurrentPID);
	//printf("ending process: %d\n", proc->pid);
	if(!proc->foreground)
		printf("[%d]\tDone\t%s\n", proc->pid, proc->name);
	parent = GetProcessByPID(proc->parent);
	if(parent->waitingPid = proc->pid)
	{
		parent->waitingPid = 0;
		if(parent->tty == currentTTY)
			awake_process(parent->pid);
	}

	proc->state = BLOCKED;
	_Sti();
	//while(1);
}

void kill(int pid)
{
	
}

void startTerminal(int pos)
{
	int i;
	for( i = 0; i < 80*25*2; i++)
	{
		terminals[pos].terminal[i]=' ';
		i++;
		terminals[pos].terminal[i] = WHITE_TXT; //0x68
	}
	terminals[pos].buffer.actual_char = BUFFER_SIZE-1;
	terminals[pos].buffer.first_char = 0;
	terminals[pos].buffer.size = 0;
	terminals[pos].PID = pos + 1;
}

void Teta(int argc, char* argv[])
{
	int i = 0, aux = 1000;
	_Sti();
	while(aux--)
	{
		_Sti();
		printf("teta0\n");
	}
	block_process(1);
	_Sti();
	while(1)
	{
		_Sti();
		printf("teta0 %d\n", i++);
	}
}

void Teta1(int argc, char* argv[])
{
	int i = 0, aux = 2000;
	_Sti();
	printf("teta1\n");
	while(aux--)
	{
		_Sti();
		printf("teta1 %d\n", i++);
	}
	//awake_TTY_proc(currentTTY);
	while(1)
	{
		_Sti();
		printf("teta1\n");
	}
}
