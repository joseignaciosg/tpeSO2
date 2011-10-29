.AUTODEPEND
!include ..\model.mak

INCLUDE = ..\include
CFLAGS = -I$(INCLUDE) -Ti$(INCLUDE) $(MODEL)
CC = bcc
LB = tlib

LIB = mtask.lib

.c.obj:
	$(CC) -c $(CFLAGS) $<
	$(LB) $(LIB) -+ $*.obj
.asm.obj:
	$(CC) -c $(CFLAGS) $<
	$(LB) $(LIB) -+ $*.obj

all:				\
	kernel.obj		\
	queue.obj 		\
	irq.obj			\
	hardvec.obj		\
	sem.obj			\
	mutex.obj		\
	monitor.obj		\
	pipe.obj		\
	msgqueue.obj	\
	math.obj		\
	hiregs.obj		\
	softint.obj		\
	callint.obj
