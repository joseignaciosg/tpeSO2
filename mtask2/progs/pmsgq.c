#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>

#include "mtask.h"
#include "mutex.h"
#include "msgqueue.h"

Mutex_t *dos;
MsgQueue_t *msgq;

void
consumer(void *arg)
{
	char *name = arg;
	long msg;

	while ( true )
	{
		if ( GetMsgQueue(msgq, &msg) )
		{
			EnterMutex(dos);
			printf("%s: %ld\n", name, msg);
			LeaveMutex(dos);
		}
	}
}

int 
main(int argc, char **argv)
{
	char buf[200];
	long msg;

	dos = CreateMutex("dos");
	msgq = CreateMsgQueue("msgq", 100, sizeof msg, false, false);

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
		if ( !strlen(buf) )
			return 0;
		LeaveMutex(dos);
		msg = atol(buf);
		PutMsgQueue(msgq, &msg);
	}
}
