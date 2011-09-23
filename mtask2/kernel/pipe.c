#include <stdlib.h>
#include <stdio.h>

#include "pipe.h"

static unsigned
get_data(Pipe_t *p, char *data, unsigned nreq, Time_t msecs)
{
	unsigned i, nbytes;

	EnterMonitor(p->monitor);
	if ( !p->avail && !WaitConditionTimed(p->cond_get, msecs) )
	{
		LeaveMonitor(p->monitor);
		return 0;
	}
	for ( nbytes = min(nreq, p->avail), i = 0 ; i < nbytes ; i++ )
	{
		*data++ = *p->head++;
		if ( p->head == p->end )
			p->head = p->buf;
	}
	p->avail -= nbytes;
	SignalCondition(p->cond_put);
	LeaveMonitor(p->monitor);
	return nbytes;
}

static unsigned
put_data(Pipe_t *p, char *data, unsigned nreq, Time_t msecs)
{
	unsigned i, nbytes;

	if ( !nreq )
		return 0;

	EnterMonitor(p->monitor);
	if ( p->avail == p->size && !WaitConditionTimed(p->cond_put, msecs) )
	{
		LeaveMonitor(p->monitor);
		return 0;
	}
	for ( nbytes = min(nreq, p->size - p->avail), i = 0 ; i < nbytes ; i++ )
	{
		*p->tail++ = *data++;
		if ( p->tail == p->end )
			p->tail = p->buf;
	}
	p->avail += nbytes;
	SignalCondition(p->cond_get);
	LeaveMonitor(p->monitor);
	return nbytes;
}

/*
--------------------------------------------------------------------------------
CreatePipe, DeletePipe - creacion y destruccion de pipes.

El parametro size determina el tamano del buffer interno. En principio puede
usarse cualquier valor; cuanto mas grande sea el buffer, mayor sera el
desacoplamiento entre los procesos que escriben y los que leen en el pipe.
Los parametros de serializacion se utilizan para especificar si deben crearse
mutexes de lectura y/o escritura para permitir que haya varios consumidores y/o
productores en el pipe.
--------------------------------------------------------------------------------
*/

Pipe_t *
CreatePipe(char *name, unsigned size, bool serialized_get, bool serialized_put)
{
	char buf[200];
	Pipe_t *p = Malloc(sizeof(Pipe_t));

	p->head = p->tail = p->buf = Malloc(p->size = size);
	p->end = p->buf + size;
	p->monitor = CreateMonitor(name);
	sprintf(buf, "Get %s", name);
	if ( serialized_get )
		p->mutex_get = CreateMutex(buf);
	p->cond_get = CreateCondition(buf, p->monitor);
	sprintf(buf, "Put %s", name);
	if ( serialized_put )
		p->mutex_put = CreateMutex(buf);
	p->cond_put = CreateCondition(buf, p->monitor);

	return p;
}

void
DeletePipe(Pipe_t *p)
{
	if ( p->mutex_get )
		DeleteMutex(p->mutex_get);
	DeleteCondition(p->cond_get);
	if ( p->mutex_put )
		DeleteMutex(p->mutex_put);
	DeleteCondition(p->cond_put);
	DeleteMonitor(p->monitor);
	Free(p->buf);
	Free(p);
}

/*
--------------------------------------------------------------------------------
GetPipe, GetPipeCond, GetPipeTimed - lectura de un pipe.

GetPipe intenta leer size bytes de un pipe, bloqueandose las veces que sea
necesario hasta poder leer todos los bytes requeridos (si el parametro get_all
es true) o alguna cantidad mayor que cero (si get_all es false). 
GetPipeTimed se comporta igual, pero puede salir prematuramente por timeout.
GetPipeCond lee los bytes que puede y retorna inmediatamente.

Estas funciones retornan la cantidad de bytes leidos. Debido al uso de un
mutex de lectura, en caso de haber mas de un proceso leyendo del pipe sus
lecturas son atomicas.
--------------------------------------------------------------------------------
*/

unsigned
GetPipe(Pipe_t *p, void *data, unsigned size, bool get_all)
{
	return GetPipeTimed(p, data, size, FOREVER, get_all);
}

unsigned
GetPipeCond(Pipe_t *p, void *data, unsigned size)
{
	return GetPipeTimed(p, data, size, 0, false);
}

unsigned
GetPipeTimed(Pipe_t *p, void *data, unsigned size, Time_t msecs, bool get_all)
{
	int n, nbytes;
	char *d;

	if ( !size || p->mutex_get && !EnterMutexTimed(p->mutex_get, msecs) )
		return 0;
	
	if ( get_all )
		for ( d = data, nbytes = 0 ; nbytes < size &&
				(n = get_data(p, d+nbytes, size-nbytes, msecs)) ; nbytes += n )
			;
	else
		nbytes = get_data(p, data, size, msecs);

	if ( p->mutex_get )
		LeaveMutex(p->mutex_get);
	return nbytes;
}

/*
--------------------------------------------------------------------------------
PutPipe, PutPipeCond, PutPipeTimed - escritura en un pipe.

PutPipe escribe size bytes en el pipe, bloqueandose las veces que sea necesario
hasta conseguir escribir todos los bytes. PutPipeTimed puede salir por timeout,
en cuyo caso escribira menos de size bytes. PutPipeCond escribe los bytes que
puede y retorna inmediatamente.

Estas funciones retornan la cantidad de bytes escritos. Debido al uso de un
mutex de escritura, en caso de haber mas de un proceso escribiendo en el pipe
sus escrituras son atomicas.
--------------------------------------------------------------------------------
*/

unsigned
PutPipe(Pipe_t *p, void *data, unsigned size)
{
	return PutPipeTimed(p, data, size, FOREVER);
}

unsigned
PutPipeCond(Pipe_t *p, void *data, unsigned size)
{
	return PutPipeTimed(p, data, size, 0);
}

unsigned
PutPipeTimed(Pipe_t *p, void *data, unsigned size, Time_t msecs)
{
	int n, nbytes;
	char *d;

	if ( !size || p->mutex_put && !EnterMutexTimed(p->mutex_put, msecs) )
		return 0;
	
	for ( d = data, nbytes = 0 ; nbytes < size &&
			(n = put_data(p, d+nbytes, size-nbytes, msecs)) ; nbytes += n )
		;

	if ( p->mutex_put )
		LeaveMutex(p->mutex_put);
	return nbytes;
}

/*
--------------------------------------------------------------------------------
AvailPipe - indica la cantidad de bytes almacenada en el pipe.
--------------------------------------------------------------------------------
*/

unsigned
AvailPipe(Pipe_t *p)
{
	return p->avail;
}
