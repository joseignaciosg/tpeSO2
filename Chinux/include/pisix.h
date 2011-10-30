/*
 * Calls (for int_80 function)
 */
#define KILL 7
#define BLOCK 8

void block_process(int pid);

void kill(int pid);

int CreateProcessAt(char* name, int (*process)(int,char**), int tty, int argc, char** argv, int stacklength, int priority, int isFront);

void clearTerminalBuffer( int ttyid);

void waitpid(int pid);

void getTerminalSize(int * size);

void getTerminalCurPos(int * curpos);

int mkfifo( char  * path, int * fd);

void semget(int * key, int initvalue, int * status);

void up(int key);

void down(int key);

void mkDir(char * newName);


