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

DESCR_INT idt[0x90]; /* IDT 144 positions*/
IDTR idtr;			 /* IDTR */

PROCESS procesos[PROCESS_QTTY];
PROCESS idle;
processList ready;
processList blocked;
static int nextPID = 1;
int CurrentPID = 0;
char stack[PROCESS_QTTY][0x400];
TTY terminals[4];
int currentTTY = 0;
int roundRobin = 0;

void startTerminal(int pos);
void set_Process_ready(PROCESS * proc);
void block_process(int pid);
void awake_TTY_proc(int TTY);
void Teta(int argc, char* argv[]);
void Teta1(int argc, char* argv[]);
void * malloc (int size);

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
	ready = blocked = NULL;
	for(i = 0; i < 4; i++)
		startTerminal(i);
	terminals[currentTTY].uninit = 0;
	CreateProcessAt("Shell", (int(*)(int, char**))shell, 0, 0, (char**)0, 0x400, 2, 1);	
	/*CreateProcessAt("Shell", (int(*)(int, char**))shell, 1, 0, (char**)0, 0x400, 2, 1);
	CreateProcessAt("Shell", (int(*)(int, char**))shell, 2, 0, (char**)0, 0x400, 2, 1);
	CreateProcessAt("Shell", (int(*)(int, char**))shell, 3, 0, (char**)0, 0x400, 2, 1);*/
	_Sti();
	while(TRUE)
	;
	return 1;
}

int CreateProcessAt(char* name, int (*process)(int,char**),int tty, int argc, char** argv, int stacklength, int priority, int isFront)
{
	int i;
	PROCESS * proc;
	for( i = 0; i < PROCESS_QTTY; i++)
	{
		if(procesos[i].free == 1)
			break;
	}
	procesos[i].pid = nextPID++;
	procesos[i].foreground=isFront;
	procesos[i].priority=priority;
	memcpy(procesos[i].name,name,str_len(name) + 1);
	procesos[i].sleep = 0;
	procesos[i].blocked = 0;
	procesos[i].tty = tty;
	procesos[i].lastCalled = 0;
	procesos[i].stacksize = stacklength;
	procesos[i].stackstart = (int)&stack[procesos[i].pid][0];
	procesos[i].free = 0;
	procesos[i].ESP = LoadStackFrame(process,argc,argv,(int)((int)&stack[procesos[i].pid][0]+stacklength-1),Cleaner);
	procesos[i].parent = 0;
	if(isFront && CurrentPID >= 1)
	{
		proc = GetProcessByPID(CurrentPID);
		/*char Men[10];*/
		proc->blocked = 2;
		procesos[i].parent = CurrentPID;
	}
	
	set_Process_ready(&procesos[i]);

	return 0;
	
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

	if(ready == NULL)
		return;

	aux = ((processNode*)ready);			/*remove process from ready list*/
	if(aux->process->pid == pid)
	{
		ready = aux->next;
		proc = aux;
	}else{
		while(aux->next != NULL && ((processNode*)aux->next)->process->pid != pid)
			aux = ((processNode*)aux->next);
		if(aux->next == NULL)			/*process is not in the ready list*/
			return;
		proc = ((processNode*)aux->next);
		aux->next = ((processNode*)aux->next)->next;
	}

	if(blocked == NULL){				/*add process to blocked list*/
		proc->process->blocked = 1;
		blocked = (processList)proc;
		return;
	}
	aux = ((processNode*)blocked);
	while(aux->next != NULL)
		aux = ((processNode*)aux->next);
	aux->next = ((processList)proc);

	return;
}

/*awakes all processes from the given tty that are blocked*/
void awake_TTY_proc(int TTY)
{
	processNode * ready_list;
	processNode * blocked_list;
	processNode * proc;
	int removed;

	if(blocked == NULL)
		return;
	
	blocked_list = ((processNode*)blocked);
	if(blocked_list->process->tty == TTY)
	{
		proc = blocked_list;
		proc->process->blocked = 0;
		blocked = blocked_list->next;
		ready_list = ((processNode*)ready);
		if(ready_list == NULL)				/*no processes in ready list*/
			ready = ((processList)proc);
		else{ 
			while(ready_list->next != NULL)
				ready_list = ((processNode*)ready_list->next);
			ready_list->next = ((processList)proc);
		}
	}
	while(blocked_list->next != NULL)
	{
		removed = 0;
		if( ((processNode*)blocked_list->next)->process->tty == TTY)
		{
			removed = 1;
			proc = ((processNode*)blocked_list->next);
			proc->process->blocked = 0;
			blocked_list->next = ((processNode*)blocked_list->next)->next;
			ready_list = ((processNode*)ready);
			if(ready_list == NULL)
				ready = ((processList)proc);
			else{
				while(ready_list->next != NULL)
					ready_list = ((processNode*)ready_list->next);
				ready_list->next = ((processList)proc);
			}
		}
		if(!removed)
			blocked_list = ((processNode*)blocked_list->next);
	}
	
	return ;
}


PROCESS * GetProcessByPID (int pid)
{
	int i;
	
	if (pid == 0)
		return &idle;

	for(i = 0; i < PROCESS_QTTY; i++)
		if (procesos[i].pid == pid)
			return &procesos[i];

	return 0;
}

int Idle(int argc, char* argv[])
{
	_Sti();
	while(1)
	;
		//printf("IDLE\n");
}

void Destroy(int PID)
{
	PROCESS* proc;
	PROCESS* padre;
	int test;
	/*char temp[10];
	char temp2[100];*/
	/*itoa2(PID,temp);*/
	proc=GetProcessByPID(PID);
	if (!proc->free)
	{
		/*memcpy2(temp2,"Killed->",8);*/
		/*test=strlen2(proc->name)+8;*/
		/*memcpy2(&temp2[8],proc->name,test-8);*/
		/*temp2[test++]='\0';*/
		/*puterr(temp2);*/
		/*tty[proc->tty].buffer.head=0;*/
		/*tty[proc->tty].buffer.tail=0;*/
		if (proc->parent!=0)
		{
			padre = GetProcessByPID(proc->parent);
			padre->blocked = 0;	
		}
		proc->free = 1;
	}
	else
		printf("Proceso Inexistente");
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
	terminals[pos].uninit = 1;
}

void Teta(int argc, char* argv[])
{
	int i = 0;
	while(1)
	printf("teta0 %d\n", i++);
}

void Teta1(int argc, char* argv[])
{
	int i = 0;
	while(1)
	printf("teta1 %d\n", i++);
}

