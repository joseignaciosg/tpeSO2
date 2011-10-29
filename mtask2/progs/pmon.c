#include <stdio.h>
#include <conio.h>
#include "mtask.h"
#include "monitor.h"

Monitor_t *mon;
Condition_t *cond;
char key;

void
consumer(void *arg)
{
	char *name = arg;

	while ( true )
	{
		EnterMonitor(mon);
		while ( !key )
			WaitCondition(cond);
		printf("%s: key %c\n", name, key);
		key = 0;
		SignalCondition(cond);
		LeaveMonitor(mon);
	}
}

int 
main(void)
{
	mon = CreateMonitor("dos");
	cond = CreateCondition("key", mon);

	Ready(CreateTask(consumer, 2000, "consumer", "consumer", DEFAULT_PRIO));

	while ( true )
	{
		EnterMonitor(mon);
		while ( key )
			WaitCondition(cond);
		key = getch();
		if ( key == 'S' || key == 's' )
			return 0;
		SignalCondition(cond);
		LeaveMonitor(mon);
	}
}
