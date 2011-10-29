#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>

#include "mtask.h"
#include "mutex.h"
#include "pipe.h"

Mutex_t *dos;
Pipe_t *pipe;
bool get_all;

void
consumer(void *arg)
{
	char *name = arg;
	char msg[100];
	unsigned size;

	while ( true )
	{
		if ( size = GetPipe(pipe, msg, sizeof msg, get_all) )
		{
			EnterMutex(dos);
			printf("%s: %.*s size %u\n", name, size, msg, size);
			LeaveMutex(dos);
		}
	}
}

int 
main(int argc, char **argv)
{
	char buf[200];
	int len;

	dos = CreateMutex("dos");
	pipe = CreatePipe("pipe", 100, false, false);
	get_all = argc == 1 ? false : atoi(argv[1]);

	Ready(CreateTask(consumer, 2000, "consumer", "consumer", DEFAULT_PRIO));

	while ( true )
	{
		EnterMutex(dos);
		if ( !kbhit() )
		{
			LeaveMutex(dos);
			continue;
		}
		gets(buf);
		if ( !(len = strlen(buf)) )
			return 0;
		LeaveMutex(dos);
		PutPipe(pipe, buf, len);
	}
}
