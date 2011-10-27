/**********************
 kc.h
**********************/
#include "defs.h"

#ifndef _kc_
#define _kc_


#define WHITE_TXT 0x07 // video attribute, white letters, black background


/* Clears the screen */
void k_clear_screen();

/* Inicializa la entrada del IDT */
void setup_IDT_entry (DESCR_INT *item, byte selector, dword offset, byte access, byte cero);


PROCESS * GetProcessByPID(int pid);
int Idle(int argc, char* argv[]);
int CreateProcessAt(char * name,int (*process)(int , char **), int tty, int argc, char ** argv, int stacklength, int priority, int isFront);
void SetupScheduler(void);
void block_process(int pid);
void awake_process(int pid);
void kill(int pid);
void sleep(int secs);
void waitpid(int pid);
void end_process(void);

#endif
