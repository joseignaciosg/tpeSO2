#include <stdlib.h>
#include <conio.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include "kernel.h"
#include "sem.h"

#define KBD_INT			9

#define FULL			'Û'
#define EMPTY			'°'
#define BUF_SIZE		40
#define TPROD			5

#define FBUF_SIZE		40
#define TDEMO			50
#define PRCONS_DEMO		"prcons.dem"

#define MSG_COL			21
#define MSG_LIN			17
#define MSG_FMT			"%s"

#define DEMO_COL		21
#define DEMO_LIN		15
#define DEMO_FMT		"%s"

#define BUF_COL			21
#define BUF_LIN			10
#define BUF_FMT			"%s"

#define TIME_COL		21
#define TIME_LIN		8
#define TIME_FMT		"Segundos:   %u"

#define PRODSTAT_COL	21
#define PRODSTAT_LIN	12
#define PRODSTAT_FMT	"Productor:  %s"

#define CONSSTAT_COL	21
#define CONSSTAT_LIN	13
#define CONSSTAT_FMT	"Consumidor: %s"

unsigned seconds;
SEMAPHORE *dos;
FILE *fdemo;
char fbuf[FBUF_SIZE+1];

char buffer[BUF_SIZE+1];
char *end = buffer + BUF_SIZE;
char *head = buffer;
char *tail = buffer;
SEMAPHORE *buf_used, *buf_free, *kbd;

TASK *prod, *cons;

void interrupt (*old_kbd)(void);

/* funciones de entrada-salida */

int 
mprint(int x, int y, char *format, ...)
{
	int n;

	WaitSem(dos, 1);
	gotoxy(x, y);
	n = vprintf(format, ...);
	clreol();
	SignalSem(dos, 1);
	return n;
}

char *
mfgets(char *buf, int size, FILE *f)
{
	char *s;

	WaitSem(dos, 1);
	s = fgets(buf, size, f);
	SignalSem(dos, 1);
	return s;
}

int
mfseek(FILE *f, long offset, int whence)
{
	int n;

	WaitSem(dos, 1);
	n = fseek(f, offset, whence);
	SignalSem(dos, 1);
	return n;
}

int
mgetch(void)
{
	int c;

	forever
	{
		WaitSem(kbd, 1);
		WaitSem(dos, 1);
		if ( kbhit() )
		{
			c = getch();
			SignalSem(dos, 1);
			return c;
		}
		SignalSem(dos, 1);
	}
}

void
put_buffer(int n)
{
	while ( n-- )
	{
		*tail++ = FULL;
		if ( tail == end )
			tail = buffer;
	}
	mprint(BUF_COL, BUF_LIN, BUF_FMT, buffer);
}

void
get_buffer(int n)
{
	while ( n-- )
	{
		*head++ = EMPTY;
		if ( head == end )
			head = buffer;
	}
	mprint(BUF_COL, BUF_LIN, BUF_FMT, buffer);
}

/* funciones auxiliares */

void interrupt
new_kbd(void)
{
	old_kbd();
	SignalSem(kbd, 1);
}

void
restore_kbd(void)
{
	setvect(KBD_INT, old_kbd);
}

char *
task_status(int status)
{
	static char buf[100];

	if ( status > READY )
	{
		sprintf(buf, "DELAY %d", status - READY);
		return buf;
	}
	if ( status < SUSPENDED )
	{
		sprintf(buf, "WAIT_EVENT %d", SUSPENDED - status);
		return buf;
	}
	switch ( status )
	{
		case READY:
			return "READY";
		case FREE:
			return "FREE";
		case SUSPENDED:
			return "SUSPENDED";
	}
	return "???";
}

void
restore_screen(void)
{
	_setcursortype(_NORMALCURSOR);
	clrscr();
}

/* procesos */

void
clock(void)
{
	forever
	{
		mprint(TIME_COL, TIME_LIN, TIME_FMT, seconds);
		Delay(18);
		++seconds;
	}
}

void
producer(int *nprod)
{
	forever
	{
		WaitSem(buf_free, *nprod);
		put_buffer(*nprod);
		SignalSem(buf_used, *nprod);
		Delay(TPROD);
	}
}

void
consumer(int *ncons)
{
	forever
	{
		mgetch();
		WaitSem(buf_used, *ncons);
		get_buffer(*ncons);
		SignalSem(buf_free, *ncons);
	}
}

void
demo(void)
{
	char *p;

	if ( !fdemo )
		return;
	forever
	{
		if ( !mfgets(fbuf, FBUF_SIZE+1, fdemo) )
		{
			mfseek(fdemo, 0L, 0);
			continue;
		}
		if ( p = strchr(fbuf, '\n') )
			*p = 0;
		mprint(DEMO_COL, DEMO_LIN, DEMO_FMT, fbuf);
		Delay(TDEMO);
	}
}

void
monitor(void)
{
	forever
	{
		mprint(PRODSTAT_COL, PRODSTAT_LIN, PRODSTAT_FMT,
			task_status(prod->status));
		mprint(CONSSTAT_COL, CONSSTAT_LIN, CONSSTAT_FMT,
			task_status(cons->status));
		Yield();
	}
}

int
main(int argc, char **argv)
{
	int nprod, ncons;

	fdemo = fopen(PRCONS_DEMO, "r");
	dos = AllocSem(1);
	kbd = AllocSem(0);

	buf_free = AllocSem(BUF_SIZE);
	buf_used = AllocSem(0);
	memset(buffer, EMPTY, BUF_SIZE);

	_setcursortype(_NOCURSOR);
	clrscr();
	atexit(restore_screen);

	old_kbd = getvect(KBD_INT);
	setvect(KBD_INT, new_kbd);
	atexit(restore_kbd);

	if ( (nprod = atoi(argv[1])) <= 0 )
		nprod = 1;
	if ( (ncons = atoi(argv[2])) <= 0 )
		ncons = 1;

	Ready(prod = CreateTask(producer, 1024, &nprod));
	Ready(cons = CreateTask(consumer, 1024, &ncons));
	Ready(CreateTask(clock, 1024, NULL));
	Ready(CreateTask(demo, 1024, NULL));
	Ready(CreateTask(monitor, 1024, NULL));

	mprint(MSG_COL, MSG_LIN, MSG_FMT,
		"Cualquier tecla para activar el consumidor,\n");
	mprint(MSG_COL, MSG_LIN+1, MSG_FMT, "Control-Break para salir ...");
	Pause();
	return 0;
}
