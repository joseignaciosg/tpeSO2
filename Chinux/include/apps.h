

#ifndef _apps_
#define _apps_


extern int last100[100];
extern int currentProcessTTY;
extern int currentTTY;
extern int CurrentPID;
extern int logPID;
extern TTY terminals[4];
extern user admin;

static int read_command();

void prioridad(int argc, char * argv[]);

void prueba(int argc, char * argv[]);

void prueba2(int argc, char * argv[]);

void top(int argc, char * argv[]);


#endif
