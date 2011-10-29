/**********************************
*
*  pisix.c implementation of pseudo posix interfaceº
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
#include "../include/process.h"
#include "../include/defs.h"

/*createProcessParam  myparam;*/

extern int nextPID;
/*extern int CurrentPID;


int CreateProcessAt(char* name, int (*process)(int,char**), int tty, int argc, char** argv, int stacklength, int priority, int isFront)
{
	PROCESS * proc;
	void * stack = malloc(stacklength);
	proc = malloc(sizeof(PROCESS));
	proc->name = (char*)malloc(15);
	proc->pid = nextPID++;
	proc->foreground = isFront;
	proc->priority = priority;
	memcpy(proc->name, name,str_len(name) + 1);
	proc->state = READY;
	proc->tty = tty;
	proc->stacksize = stacklength;
	proc->stackstart = (int)stack;
	proc->ESP = LoadStackFrame(process,argc,argv,(int)(stack + stacklength - 1), end_process);
	proc->parent = CurrentPID;
	proc->waitingPid = 0;
	proc->sleep = 0;
	proc->acum = priority + 1;
	set_Process_ready(proc);
	return proc->pid;
}
*/

void block_process(int pid){
	_int_79_caller(BLOCK, pid);
}

void kill(int pid){
	_int_79_caller(KILL, pid);
}

int CreateProcessAt(char* name, int (*process)(int,char**), int tty, int argc, char** argv, int stacklength, int priority, int isFront){
	createProcessParam * param = (createProcessParam *)malloc(sizeof(createProcessParam));
	strcopy( param->name, name, str_len(name) );
	param->process = process;
	param->tty = tty;
	param->argc = argc;
	param->argv = argv;
	param->stacklength = stacklength;
	param->priority = priority;
	param->isFront = isFront;
	_int_79_caller(CREATE,param);
	return nextPID++;
}

void clearTerminalBuffer( int ttyid){
	_int_79_caller(CLEAR_TERM,ttyid);
}
