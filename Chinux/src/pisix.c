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


extern int nextPID;




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

void waitpid(int pid)
{
	_int_79_caller(WAIT_PID,pid);
}

void getTerminalSize(int * size)
{
	_int_79_caller(TERM_SIZE,size);
}

