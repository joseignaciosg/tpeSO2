#include <dos.h>
#include <conio.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "mtask.h"
#include "mutex.h"
#include "sem.h"

#define KBD_IRQ			1

Semaphore_t *kbd;
Mutex_t *mut;
double d;

static int
mgetch(void)
{
	int c;

	while ( true )
	{
		EnterMutex(mut);
		if ( kbhit() )
		{
			c = getch();
			LeaveMutex(mut);
			return c;
		}
		LeaveMutex(mut);
		WaitSem(kbd);
	}
}

static void 
new_kbd(unsigned irq)
{
	OldInterrupt(irq);
	SignalSem(kbd);
}

static int
mprintf(char *format, ...)
{
	int n;

	EnterMutex(mut);
	n = vprintf(format, ...);
	LeaveMutex(mut);
	return n;
}

static double
do_math(char *who)
{
	double dd;

	dd = sqrt(((1.2345 * 1.2345) + (1.2345 * 1.2345)) / 2.0);
	if ( dd != d )
		mprintf("%s: %f %f\n", who, dd, d);
	return dd;
}

#pragma argsused
void
h(void *arg)
{
	while ( 1 )
		do_math("task");
}

int
main(int argc, char **argv)
{
	Task_t *other;
	int i = 0;

	mut = CreateMutex("mut");
	kbd = CreateSem("kbd", 0);
	d = do_math("main");

	SetHandler(KBD_IRQ, new_kbd);

	other = CreateTask(h, 2000, NULL, "task h", DEFAULT_PRIO-1);

	if ( argc > 1 && atoi(argv[1]) )
		ProtectMath(CurrentTask());
	if ( argc > 2 && atoi(argv[2]) )
		ProtectMath(other);

	Ready(other);
	printf("main: _8087=%d\n", _8087);
	while ( true )
	{
		if ( mgetch() == 's' )
		{
			Delay(100);
			return 0;
		}
		mprintf("main: do_math %d\n", ++i);
		do_math("main");
	}
}
