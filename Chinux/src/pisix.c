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
#include "../include/shell.h"
#include "../include/utils.h"
#include "../include/process.h"
#include "../include/defs.h"
#include "../include/pisix.h"


extern int nextPID;




void block_process(int pid){
	_int_79_caller(BLOCK, pid);
}

void kill(int pid){
	_int_79_caller(KILL, pid);
}

int CreateProcessAt(char* name, int (*process)(int,char**), int tty, int argc, char** argv, int stacklength, int priority, int isFront)
{
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

void clearTerminalBuffer( int ttyid)
{
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

void getTerminalCurPos(int * curpos)
{
	_int_79_caller(TERM_CURPOS,curpos);
}

/*void getCurrentTTY(int currtty)
{
	_int_79_caller(CURR_TTY,currtty);
}*/

int mkfifo(  int * fd )
{
	fifoStruct * param = malloc(sizeof(fifoStruct));
	/*param->name = malloc(str_len(name));
	memcpy(param->name,name,str_len(name));
	printf("mkfifo: %s\n", param->name);*/
	_int_79_caller(MK_FIFO,&param);
	fd[0] = param->fd1;
	fd[1] = param->fd2;
}

void rmfifo( int * fd ){
	fifoStruct * param = malloc(sizeof(fifoStruct));
	param->fd1 = fd[0];
	param->fd2 = fd[1];
	_int_79_caller(RM_FIFO,&param);
}


void semget(int * key, int initvalue, int * status){
	semItem * param = malloc(sizeof(semItem));
	param->value = initvalue;
	_int_79_caller(SEM_GET,&param);
	(*status) = param->status;
	(*key) = param->key;
}

void semrm(int key){
	_int_79_caller(SEM_RM,key);
}

void up(int key){
	_int_79_caller(SEM_UP,key);
}

void down(int key){
	_int_79_caller(SEM_DOWN,key);
}

void mkDir(char * newName){
	_int_79_caller(MK_DIR,newName);
}

void ls(char * path){
	_int_79_caller(LS_COM,path);
}

void rm( char * path ){
	_int_79_caller(RM_COM,path);
}

void touch( char * filename ){
	_int_79_caller(TOUCH_COM,filename);
}

void cat( char * filename ){
	_int_79_caller(CAT_COM,filename);
}

void cd(char * path){
	_int_79_caller(CD_COM,path);
}

void link(char * path1, char * path2){
	link_struct * param = malloc(sizeof(link_struct));
	memcpy(param->path1,path1,str_len(path1));
	memcpy(param->path2,path2,str_len(path2));
	_int_79_caller(CD_COM,param);
}

int creat(char * filename,int mode){
	creat_param * param = malloc(sizeof(creat_param));
	param->filename = malloc(str_len(filename));
	memcpy(param->filename,filename,str_len(filename));
	param->mode = mode;
	_int_79_caller(CREAT_COM,param);
}



