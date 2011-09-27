/********************************** 
*
*  kernel.c
*  	Galindo, Jose Ignacio
*  	Homovc, Federico
*		ITBA 2011
*
***********************************/


/***	Proyect Includes	***/
#include "../include/kasm.h"
#include "../include/kernel.h"

#include "../include/kc.h"
#include "../include/shell.h"

extern KEY_BUFFER keybuffer;
DESCR_INT idt[0x90]; /* IDT de 144 entradas*/
IDTR idtr;			 /* IDTR */

PROCESS procesos[10];
PROCESS idle;
static int nextPID=1;
int CurrentPID=0;
char stack[10][0x400];
TTY terminals[4];
int currentTTY = 0;
enum ttyFocus {ONE=0, TWO, THREE, FOUR};

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
kmain()
Starting point of the whole OS
*************************************************/
int
kmain()      //   /usr/share/bochs/.bochsrc
{
	int i, j;
	_Cli();
	initializeKeyBuffer();
	k_clear_screen();

	initializeIDT();
	unmaskPICS();
	SetupScheduler();
	for(i = 0; i < 4; i++)
	{
		j = 0;
		while(j < (80*25*2))
		{
			terminals[i].terminal[j]=' ';
			j++;
			terminals[i].terminal[j] = WHITE_TXT; //0x68
			j++;
		}
		terminals[i].uninit = 1;
	}
	terminals[0].uninit = 0;
	
	//CreateProcessAt("Teta 0",(int(*)(int, char**))Teta,0,0,(char**)0,0x400,2,1); //tty0
	//CreateProcessAt("Teta 0",(int(*)(int, char**))Teta1,1,0,(char**)0,0x400,2,1); //tty1
	//CreateProcessAt("Teta 0",(int(*)(int, char**))Teta2,2,0,(char**)0,0x400,2,1); //tty2
	CreateProcessAt("Teta 0",(int(*)(int, char**))shell,0,0,(char**)0,0x400,2,1);
	_Sti();
	while(1)
		;	

	return 1;
}

void CreateProcessAt(char* name, int (*process)(int,char**),int tty, int argc, char** argv, int stacklength, int priority, int isFront)
{
	char* video =(char*) 0xb8000;
	int i;
	PROCESS* proc;
	for(i=0;i<64;i++)
	{
		if(procesos[i].free==1)
			break;
	}
	procesos[i].pid=GetPID();
	procesos[i].foreground=isFront;
	procesos[i].priority=priority;
	/*memcpy2(procesos[i].name,name,strlen2(name)+1);*/
	procesos[i].sleep=0;
	procesos[i].blocked=0;
	procesos[i].tty=tty;
	procesos[i].lastCalled=0;
	procesos[i].stacksize=stacklength;
	procesos[i].stackstart=(int)&stack[procesos[i].pid][0];
	procesos[i].free = 0;
	procesos[i].ESP = LoadStackFrame(process,argc,argv,(int)((int)&stack[procesos[i].pid][0]+stacklength-1),Cleaner);
	procesos[i].parent = 0;
	if(isFront && CurrentPID >= 1)
	{
		proc = GetProcessByPID(CurrentPID);
		/*char Men[10];*/
		proc->blocked=2;
		procesos[i].parent=CurrentPID;
	}
	//video[2080]=procesos[i].pid+0x30;
	
}

int LoadStackFrame(int(*process)(int,char**),int argc,char** argv, int bottom, void(*cleaner)())
{
	STACK_FRAME* frame = (STACK_FRAME*)(bottom-sizeof(STACK_FRAME));
	frame->EBP=0;
	frame->EIP=(int)process;
	frame->CS=0x08;
	
	frame->EFLAGS=0;
	frame->retaddr=cleaner;
	frame->argc=argc;
	frame->argv=argv;
	return (int)frame;
}

int GetPID(void)
{
	return nextPID++;
}

PROCESS* GetProcessByPID (int pid)
{
	char* video=(char*)0xb8000;
	int i;
	//video[2080+contador*2]=pid+0x30;
	
	if (pid==0)
	{
		return &idle;
	}
	for(i=0;i<64;i++)
	{
		if (procesos[i].pid== pid)
		{
			return &procesos[i];
		}
	}
	return 0;
}

int Idle(int argc, char* argv[])
{
	_Sti();
	while(1)
	{
	}
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

void Teta2(int argc, char* argv[])
{
	int i = 0;
	while(1)
	printf("teta2 %d\n", i++);
}
