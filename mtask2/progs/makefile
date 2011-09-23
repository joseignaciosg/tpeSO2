.AUTODEPEND
!include ..\model.mak

CC = bcc
CFLAGS = -3 -I..\include $(MODEL)
MTASK = ..\kernel\mtask.lib

.c.obj:
	$(CC) -c $(CFLAGS) $< 

all: camino.exe pmon.exe psend.exe camino1.exe pmath.exe ppipe.exe \
	 pmsgq.exe camino2.exe filo.exe sfilo.exe

pmon = pmon.obj $(MTASK)
psend = psend.obj $(MTASK)
pmath = pmath.obj $(MTASK)
ppipe = ppipe.obj $(MTASK)
pmsgq = pmsgq.obj $(MTASK)
camino = camino.obj $(MTASK)
camino1 = camino1.obj $(MTASK)
camino2 = camino2.obj $(MTASK)
filo = filo.obj $(MTASK)
sfilo = sfilo.obj $(MTASK)

pmon.exe: $(pmon)
	$(CC) $(CFLAGS) $(pmon)
psend.exe: $(psend)
	$(CC) $(CFLAGS) $(psend)
pmath.exe: $(pmath)
	$(CC) $(CFLAGS) $(pmath)
ppipe.exe: $(ppipe)
	$(CC) $(CFLAGS) $(ppipe)
pmsgq.exe: $(pmsgq)
	$(CC) $(CFLAGS) $(pmsgq)
camino.exe: $(camino)
	$(CC) $(CFLAGS) $(camino)
camino1.exe: $(camino1)
	$(CC) $(CFLAGS) $(camino1)
camino2.exe: $(camino2)
	$(CC) $(CFLAGS) $(camino2)
filo.exe: $(filo)
	$(CC) $(CFLAGS) $(filo)
sfilo.exe: $(sfilo)
	$(CC) $(CFLAGS) $(sfilo)
