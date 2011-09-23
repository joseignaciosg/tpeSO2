#include <stdlib.h>
#include <conio.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include "mtask.h"
#include "sem.h"
#include "mutex.h"

#define KBD_IRQ			1

#define PASS_MAX		10

#define LEFTCAR			'<'
#define RIGHTCAR		'>'
#define EMPTY			'°'
#define BARRIER			'|'
#define SENSOR			'*'
#define NOTHING			' '

#define RIGHTCOL		80
#define SENSOR1_POS		9
#define BARRIER_POS		19
#define ONEWAY_START	20
#define ONEWAY_END		39
#define SENSOR2_POS		40
#define ROAD_END		59
#define ROAD_LEN		(ROAD_END+1)

#define TDELAY			100

#define	BASELINE		13
#define MSGLINE			18
#define	OFFSET			((RIGHTCOL-ROAD_LEN)/2)

#define NORMAL_VIDEO	0x07
#define INVERSE_VIDEO	0x70

#define forever while(true)

typedef enum { LEFTBOUND, RIGHTBOUND } DIRECTION;
typedef enum { IDLE, LEFTBOUND_BUSY, LEFTBOUND_DRAINING,
				RIGHTBOUND_BUSY, RIGHTBOUND_DRAINING } ROADSTATE;
typedef enum { LEFTBOUND_IN, LEFTBOUND_OUT, LEFTBOUND_BARRIER,
				RIGHTBOUND_IN, RIGHTBOUND_OUT, RIGHTBOUND_BARRIER } MESSAGE;

static char road[2][ROAD_LEN];
static Mutex_t *dos;
static Semaphore_t *kbd;
static Task_t *control_task;
static int pass_max = PASS_MAX;

/* funciones de entrada-salida */

static void 
mputch(int x, int y, int c, bool inverse_video)
{
	EnterMutex(dos);
	gotoxy(x, y);
	if ( inverse_video )
		textattr(INVERSE_VIDEO);
	putch(c);
	if ( inverse_video )
		textattr(NORMAL_VIDEO);
	LeaveMutex(dos);
}

static int
mgetch(void)
{
	int c;

	forever
	{
		WaitSem(kbd);
		EnterMutex(dos);
		if ( kbhit() )
		{
			c = getch();
			LeaveMutex(dos);
			return c;
		}
		LeaveMutex(dos);
	}
}

int 
mprint(int x, int y, char *format, ...)
{
	int n;

	EnterMutex(dos);
	gotoxy(x, y);
	n = vprintf(format, ...);
	clreol();
	LeaveMutex(dos);
	return n;
}

/* Funciones de manejo del camino */

static int
get_road(DIRECTION dir, int pos)
{
	return road[dir][pos];
}

static void
draw(DIRECTION dir, int pos, int value, int delta_y, bool inverse_video)
{
	int x, y;

	if ( dir == LEFTBOUND )
	{
		x = OFFSET + ROAD_END - pos;
		y = pos < ONEWAY_START || pos > ONEWAY_END ?
			BASELINE-delta_y : BASELINE;
	}
	else
	{
		x = OFFSET + pos;
		y = pos < ONEWAY_START || pos > ONEWAY_END ?
			BASELINE+delta_y : BASELINE;
	}
	mputch(x, y, value, inverse_video);
}

static void
set_road(DIRECTION dir, int pos, int value, bool inverse_video)
{
	road[dir][pos] = value;
	draw(dir, pos, value, 1, inverse_video);
}

static void
set_border(DIRECTION dir, int pos, int value)
{
	draw(dir, pos, value, 2, false);
}

static void
open_barrier(DIRECTION dir)
{
	set_road(dir, BARRIER_POS, EMPTY, false);
	set_border(dir, BARRIER_POS, SENSOR);
}

static void
close_barrier(DIRECTION dir)
{
	Atomic();
	while ( get_road(dir, BARRIER_POS) != EMPTY )
		Yield();
	set_road(dir, BARRIER_POS, BARRIER, false);
	Unatomic();
	set_border(dir, BARRIER_POS, NOTHING);
}

static void
init_road(void)
{
	int i;

	for ( i = 0 ; i < ROAD_LEN ; i++ )
	{
		set_road(LEFTBOUND, i, EMPTY, false);
		set_road(RIGHTBOUND, i, EMPTY, false);
	}
	set_border(LEFTBOUND,  SENSOR1_POS, SENSOR);
	set_border(LEFTBOUND,  SENSOR2_POS, SENSOR);
	set_border(RIGHTBOUND, SENSOR1_POS, SENSOR);
	set_border(RIGHTBOUND, SENSOR2_POS, SENSOR);
}

/* Procesos */

#pragma argsused
static void
control(void *arg)
{
	int leftbound_cars = 0, rightbound_cars = 0, in_road = 0, passed = 0;
	ROADSTATE state = IDLE;
	MESSAGE msg;
	unsigned size;

	close_barrier(LEFTBOUND);
	close_barrier(RIGHTBOUND);
	forever
	{
		size = sizeof msg;
		Receive(NULL, &msg, &size);

		switch ( msg )
		{
			case LEFTBOUND_IN:
				++leftbound_cars;
				break;
			case RIGHTBOUND_IN:
				++rightbound_cars;
				break;
			case LEFTBOUND_BARRIER:
				--leftbound_cars;
				++in_road;
				if ( passed < pass_max )
					++passed;
				break;
			case RIGHTBOUND_BARRIER:
				--rightbound_cars;
				++in_road;
				if ( passed < pass_max )
					++passed;
				break;
			case LEFTBOUND_OUT:
			case RIGHTBOUND_OUT:
				--in_road;
				break;
		}
		switch ( state )
		{
			case IDLE:
				if ( leftbound_cars )
				{
					open_barrier(LEFTBOUND);
					passed = 0;
					state = LEFTBOUND_BUSY;
				}
				else if ( rightbound_cars )
				{
					open_barrier(RIGHTBOUND);
					passed = 0;
					state = RIGHTBOUND_BUSY;
				}
				break;
			case LEFTBOUND_BUSY:
				if ( !leftbound_cars && !rightbound_cars && !in_road )
				{
					close_barrier(LEFTBOUND);
					state = IDLE;
				}
				else if ( rightbound_cars && (!leftbound_cars ||
							passed == pass_max) )
				{
					close_barrier(LEFTBOUND);
					if ( in_road )
						state = LEFTBOUND_DRAINING;
					else
					{
						open_barrier(RIGHTBOUND);
						passed = 0;
						state = RIGHTBOUND_BUSY;
					}
				}
				break;
			case LEFTBOUND_DRAINING:
				if ( !in_road )
				{
					open_barrier(RIGHTBOUND);
					passed = 0;
					state = RIGHTBOUND_BUSY;
				}
				break;
			case RIGHTBOUND_BUSY:
				if ( !leftbound_cars && !rightbound_cars && !in_road )
				{
					close_barrier(RIGHTBOUND);
					state = IDLE;
				}
				else if ( leftbound_cars && (!rightbound_cars ||
							passed == pass_max) )
				{
					close_barrier(RIGHTBOUND);
					if ( in_road )
						state = RIGHTBOUND_DRAINING;
					else
					{
						open_barrier(LEFTBOUND);
						passed = 0;
						state = LEFTBOUND_BUSY;
					}
				}
				break;
			case RIGHTBOUND_DRAINING:
				if ( !in_road )
				{
					open_barrier(LEFTBOUND);
					passed = 0;
					state = LEFTBOUND_BUSY;
				}
				break;
		}
	}
}

static void
car(void *arg)
{
	DIRECTION *dir = arg;
	int pos;
	int carsymbol = *dir == LEFTBOUND ? LEFTCAR : RIGHTCAR;
	MESSAGE car_in = *dir == LEFTBOUND ? LEFTBOUND_IN : RIGHTBOUND_IN; 
	MESSAGE car_barrier = *dir == LEFTBOUND ?
							LEFTBOUND_BARRIER : RIGHTBOUND_BARRIER; 
	MESSAGE car_out = *dir == LEFTBOUND ? LEFTBOUND_OUT : RIGHTBOUND_OUT; 


	for ( pos = 0 ; pos < ROAD_LEN ; pos++ )
	{
		Atomic();
		while ( get_road(*dir, pos) != EMPTY )
			Yield();
		if ( pos < ROAD_END && get_road(*dir, pos+1) != EMPTY )
			Delay(TDELAY);
		while ( get_road(*dir, pos) != EMPTY )
			Yield();
		set_road(*dir, pos, carsymbol, true);
		if ( pos == SENSOR1_POS )
			Send(control_task, &car_in, sizeof car_in);
		else if ( pos == BARRIER_POS )
			Send(control_task, &car_barrier, sizeof car_barrier);
		else if ( pos == SENSOR2_POS )
			Send(control_task, &car_out, sizeof car_out);
		if ( pos )
			set_road(*dir, pos-1, EMPTY, false);
		Unatomic();
		Delay(TDELAY);
	}
	set_road(*dir, ROAD_END, EMPTY, false);
}

/* Otras funciones */

static void
restore_screen(void)
{
	_setcursortype(_NORMALCURSOR);
	clrscr();
}

static void
new_kbd(unsigned irq)
{
	OldInterrupt(irq);
	SignalSem(kbd);
}

static void
send_car(DIRECTION dir)
{
	static DIRECTION left = LEFTBOUND, right = RIGHTBOUND;

	EnterMutex(dos);	// En modelo large, malloc puede llamar a DOS
	Ready(CreateTask(car, 1024, dir == LEFTBOUND ? &left : &right, 
			"car", DEFAULT_PRIO));
	LeaveMutex(dos);
}

int
main(int argc, char **argv)
{
	if ( argc > 1 )
		pass_max = atoi(argv[1]);

	dos = CreateMutex("dos");
	kbd = CreateSem("key", 0);

	_setcursortype(_NOCURSOR);
	clrscr();
	atexit(restore_screen);

	SetHandler(KBD_IRQ, new_kbd);

	init_road();

	Ready(control_task = 
			CreateTask(control, 1024, NULL, "control", DEFAULT_PRIO));

	mprint(OFFSET, MSGLINE,		"I: auto hacia la izquierda");
	mprint(OFFSET, MSGLINE+1,	"D: auto hacia la derecha");
	mprint(OFFSET, MSGLINE+2,	"S: salir");

	forever
		switch ( mgetch() )
		{
			case 'I':
			case 'i':
				send_car(LEFTBOUND);
				break;
			case 'D':
			case 'd':
				send_car(RIGHTBOUND);
				break;
			case 'S':
			case 's':
				Delay(100);
				EnterMutex(dos);
				return 0;
		}
}
