#ifndef PIPE_H
#define PIPE_H

#include "mutex.h"
#include "monitor.h"

typedef struct
{
	Mutex_t *		mutex_get;
	Mutex_t *		mutex_put;
	Monitor_t *		monitor;
	Condition_t *	cond_get;
	Condition_t *	cond_put;
	unsigned		size;
	unsigned		avail;
	char *			buf;
	char *			head;
	char *			tail;
	char *			end;
}
Pipe_t;

Pipe_t *			CreatePipe(char *name, unsigned size, bool serialized_get,
									bool serialized_put);
void				DeletePipe(Pipe_t *p);
unsigned			GetPipe(Pipe_t *p, void *data, unsigned size,
						bool get_all);
unsigned			GetPipeCond(Pipe_t *p, void *data, unsigned size);
unsigned			GetPipeTimed(Pipe_t *p, void *data, unsigned size, 
						Time_t msecs, bool get_all);
unsigned			PutPipe(Pipe_t *p, void *data, unsigned size);
unsigned			PutPipeCond(Pipe_t *p, void *data, unsigned size);
unsigned			PutPipeTimed(Pipe_t *p, void *data, unsigned size, 
						Time_t msecs);
unsigned			AvailPipe(Pipe_t *p);

#endif
