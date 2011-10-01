#ifndef MSGQUEUE_H
#define MSGQUEUE_H

#include "mutex.h"
#include "sem.h"

typedef struct
{
	Mutex_t *		mutex_get;
	Mutex_t *		mutex_put;
	Semaphore_t *	sem_get;
	Semaphore_t *	sem_put;
	unsigned		msg_size;
	char *			buf;
	char *			head;
	char *			tail;
	char *			end;
}
MsgQueue_t;

MsgQueue_t *		CreateMsgQueue(char *name, unsigned msg_max, unsigned msg_size,
									bool serialized_get, bool serialized_put);
void				DeleteMsgQueue(MsgQueue_t *mq);
bool				GetMsgQueue(MsgQueue_t *mq, void *msg);
bool				GetMsgQueueCond(MsgQueue_t *mq, void *msg);
bool				GetMsgQueueTimed(MsgQueue_t *mq, void *msg, Time_t msecs);
bool				PutMsgQueue(MsgQueue_t *mq, void *msg);
bool				PutMsgQueueCond(MsgQueue_t *mq, void *msg);
bool				PutMsgQueueTimed(MsgQueue_t *mq, void *msg, Time_t msecs);
unsigned			AvailMsgQueue(MsgQueue_t *mq);

#endif
