#include <stdio.h>
#include <conio.h>
#include <string.h>

#include "mtask.h"
#include "mutex.h"

Mutex_t *dos;

void
consumer(void *arg)
{
	char *name = arg;
	char msg[100];
	Task_t *from;
	unsigned size;

	while ( true )
	{
		from = NULL;
		size = sizeof msg;
		if ( Receive(&from, msg, &size) )
		{
			EnterMutex(dos);
			printf("%s: %s from %s size %u\n", name, msg, from->name, size);
			LeaveMutex(dos);
		}
	}
}

int 
main(void)
{
	char buf[200];
	Task_t *cons;

	dos = CreateMutex("dos");

	Ready(cons =
		CreateTask(consumer, 2000, "consumer", "consumer", DEFAULT_PRIO));

	while ( true )
	{
		EnterMutex(dos);
		if ( !kbhit() )
		{
			LeaveMutex(dos);
			continue;
		}
		gets(buf);
		if ( !strlen(buf) )
			return 0;
		LeaveMutex(dos);
		Send(cons, buf, strlen(buf)+1);
	}
}
