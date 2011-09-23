#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <string.h>
#include <math.h>
#include "mtask.h"
#include "mutex.h"
#include "monitor.h"

#define NF						5
#define MIN_THINK				2000
#define MAX_THINK				6000
#define MIN_EAT					2000
#define MAX_EAT					6000

#define COLOR_NORMAL			LIGHTGRAY
#define COLOR_THINKING			WHITE
#define COLOR_HUNGRY			YELLOW
#define COLOR_EATING			LIGHTRED
#define COLOR_FREE				WHITE
#define COLOR_USED				LIGHTRED

typedef struct
{
	int x;
	int y;
}
point;

Mutex_t *mut;					// protege DOS y la función rand()
Monitor_t *mon;					// monitor de acceso a los tenedores
bool in_use[NF];				// tenedores usados
Condition_t *cond[NF];			// una variable de condicion por filosofo

int
prev(int n)
{
	return n == 0 ? NF-1 : n-1;
}

int
next(int n)
{
	return n == NF-1 ? 0 : n+1;
}

#define left_fork(i)			(i)
#define right_fork(i)			next(i)
#define left_phil(i)			prev(i)
#define right_phil(i)			next(i)

point pos[2*NF];				// posiciones en pantalla de los filosofos y los tenedores

#define phil_position(i)		(pos[2*(i)+1])
#define fork_position(i)		(pos[2*(i)])

int 
mprint(point where, char *msg, unsigned color)
{
	int n;
	static int width;

	EnterMutex(mut);
	if ( (n = strlen(msg)) > width )
		width = n;
	gotoxy(where.x, where.y);
	textcolor(color);
	n = cprintf("%-*.*s", width, width, msg);
	LeaveMutex(mut);

	return n;
}

void 
random_delay(int d1, int d2)
{
	Time_t t;

	EnterMutex(mut);
	t = d1 + rand() % (d2-d1);
	LeaveMutex(mut);

	Delay(t);
}

void
think(int i)
{
	mprint(phil_position(i), "THINKING", COLOR_THINKING);
	random_delay(MIN_THINK, MAX_THINK);
}

void
become_hungry(int i)
{
	mprint(phil_position(i), "HUNGRY", COLOR_HUNGRY);
}

void
eat(int i)
{
	mprint(phil_position(i), "EATING", COLOR_EATING);
	random_delay(MIN_EAT, MAX_EAT);
}

bool
forks_avail(int i)
{
	return !in_use[left_fork(i)] && !in_use[right_fork(i)];
}

void
take_forks(int i)
{
	in_use[left_fork(i)] = in_use[right_fork(i)] = true;
	mprint(fork_position(left_fork(i)), "used", COLOR_USED);
	mprint(fork_position(right_fork(i)), "used", COLOR_USED);
}

void
leave_forks(int i)
{
	in_use[left_fork(i)] = in_use[right_fork(i)] = false;
	mprint(fork_position(left_fork(i)), "free", COLOR_FREE);
	mprint(fork_position(right_fork(i)), "free", COLOR_FREE);
}

#pragma argsused
void
philosopher(void *arg)
{
	int i;
	unsigned len = sizeof i;
	int left, right;

	Receive(NULL, &i, &len);
	left = left_phil(i);
	right = right_phil(i);

	while ( true )
	{
		think(i);
		become_hungry(i);
		EnterMonitor(mon);
		if ( !forks_avail(i) )
			WaitCondition(cond[i]);
		take_forks(i);
		LeaveMonitor(mon);
		eat(i);
		EnterMonitor(mon);
		leave_forks(i);
		if ( forks_avail(left) )
			SignalCondition(cond[left]);
		if ( forks_avail(right) )
			SignalCondition(cond[right]);
		LeaveMonitor(mon);
	}
}

void 
set_positions(void)
{
	int i;

	for ( i = 0 ; i < 2*NF ; i++ )
	{
		pos[i].x = 35 + 20 * cos(i * M_PI/NF);
		pos[i].y = 14 - 10 * sin(i * M_PI/NF);
	}
}

void 
restore(void)
{
	textcolor(COLOR_NORMAL);
	_setcursortype(_NORMALCURSOR);
	clrscr();
}

int 
main(void)
{
	int i;
	Task_t *t;

	clrscr();
	_setcursortype(_NOCURSOR);
	atexit(restore);

	puts("Oprima una tecla para salir...");

	// Inicializar estructuras de datos
	mut = CreateMutex(NULL);
	mon = CreateMonitor(NULL);
	set_positions();
	for ( i = 0 ; i < NF ; i++ )
	{
		mprint(fork_position(i), "free", COLOR_FREE);
		cond[i] = CreateCondition(NULL, mon);
	}

	// Disparar procesos
	srand(time(NULL));
	for ( i = 0 ; i < NF ; i++ )
	{
		t = CreateTask(philosopher, 2000, NULL, NULL, DEFAULT_PRIO);
		Ready(t);
		Send(t, &i, sizeof i);
	}

	while ( true )
	{
		EnterMutex(mut);
		if ( kbhit() )
			break;
		LeaveMutex(mut);
		Delay(1000);
	}
	return 0;
}
