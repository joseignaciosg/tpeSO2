/**********************************
 *
 *  apps.c
 *  	Galindo, Jose Ignacio
 *  	Homovc, Federico
 *	Loreti, Nicolas
 *		ITBA 2011
 *
 ***********************************/

/***	Project Includes	***/
#include "../include/defs.h"
#include "../include/shell.h"
#include "../include/stdio.h"
#include "../include/tests.h"
#include "../include/utils.h"
#include "../include/keyboard.h"
#include "../include/video.h"
#include "../include/fs.h"
#include "../include/atadisk.h"
#include "../include/apps.h"
#include "../include/kasm.h"
#include "../include/pisix.h"

/***	Module Defines	***/
#define STO_MAX  100

processCPU top100[100];


void prioridad(int argc, char * argv[])
{
	_Sti();
	while(TRUE)
	;
}

void prueba2(int argc, char * argv[])
{
	int i = 50000000;
	CreateProcessAt("Prueba", (int(*)(int, char**))prueba, currentProcessTTY, 0, (char**)0, 0x400, 2, 1);
	CreateProcessAt("Prueba", (int(*)(int, char**))prueba, currentProcessTTY, 0, (char**)0, 0x400, 2, 1);
	CreateProcessAt("Prueba", (int(*)(int, char**))prueba, currentProcessTTY, 0, (char**)0, 0x400, 2, 1);
	_Sti();
	printf("prueba2\n");
	while(i--)
		;

	return;
}

void prueba(int argc, char * argv[])
{
	_Sti();
	printf("prueba\n");
	while(TRUE)
		;
	return;
}


void top(int argc, char * argv[])
{
	int i, j, length, pos;
	PROCESS * proc;
	_Sti();
	while(TRUE)
	{
		for(i = 0; i < 100; i++)
		{
			top100[i].pid = -1;
			top100[i].cpu = 0;
		}
		pos = 0;
		for(i = 0; i < 100; i++)
		{
			if(last100[i] == -1)
			{
				if(top100[pos].pid == -1)
					pos = 0;
				top100[pos].cpu++;
				pos++;
			}
			else{
				j = 0;
				while(top100[j].pid != last100[i] && top100[j].pid != -1)
					j++;
				top100[j].pid = last100[i];
				top100[j].cpu++;
			}
		}
		printf("Process Name        cpu       PID\n");
		for(j = 0; top100[j].pid != -1; j++)
		{
			length = 20;
			proc = (PROCESS *) GetProcessByPID(top100[j].pid);
			printf("%s", proc->name);
			length -= str_len(proc->name);
			while(length--)
				putc(' ');
			printf("%d         %d\n", top100[j].cpu, top100[j].pid);
		}
		sleep(3);
		k_clear_screen();
	}
}

void fifo_writer_test(int argc, char * argv[]){
	int * fd = (int *)malloc(2 * sizeof(int));
	char * buff = (char *)malloc(9);
	mkfifo(fd);
	CreateProcessAt("fifo_reader", (int(*)(int, char**))fifo_reader_test, 1, (int)fd, 0, 0x400, 2, 1);
	sleep(3);
	write_fifo(fd[0],"hello   ",8);
	sleep(3);
	write_fifo(fd[0],"how are ",8);
	sleep(3);
	write_fifo(fd[0],"you?    ",8);
	read_fifo(fd[1],buff,9);
	printf("fifo_writer receives: %s\n", buff);
	rmfifo(fd);


}

void fifo_reader_test(int argc, char * argv[]){
	int * fd = (int *)argc;
	char * buff = (char *)malloc(8);
	int i=0;
	while(i<3){
		read_fifo(fd[0],buff, 8);
		printf("fifo_reader receives: %s\n",buff);
		i++;
	}
	write_fifo(fd[1],"just fine",9);
	return;
}

