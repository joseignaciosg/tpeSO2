/********************************** 
 *
 *  pisix.h
 *  	Galindo, Jose Ignacio
 *  	Homovc, Federico
 *  	Loreti, Nicolas
 *		ITBA 2011
 *
 ***********************************/

/*
 * Calls (for int_80 function)
 */
#define KILL 7
#define BLOCK 8

/*
 * block_process
 *
 *descrip: blocks a process with a given pid
 *#pid: pid of the process to block
 * */
void block_process(int pid);

/*
 * kill
 *
 *descrip: kill a process with a given pid
 *#pid: pid of the process to kill
 * */
void kill(int pid);

/*
 * CreateProcessAt
 *
 *descrip: creates a process with a given name in a given tty
 *#pid: pid of the process to kill
 * */
int CreateProcessAt(char* name, int (*process)(int,char**), int tty, int argc, char** argv, int stacklength, int priority, int isFront);

/*
 * clearTerminalBuffer
 *
 *descrip: clears the terminal buffer
 *#ttyid: id of the terminal whose buffer is needed to be cleared
 * */
void clearTerminalBuffer( int ttyid);

/*
 * waitpid
 *
 *descrip: waits for the process with a given pid
 *#pid: pid of the process to wait for
 * */
void waitpid(int pid);

/*
 * clearTerminalBuffer
 *
 *descrip: clears the terminal buffer
 *#ttyid: id of the terminal whose buffer is needed to be cleared
 * */
void getTerminalSize(int * size);

/*
 * getTerminalCurPos
 *
 *descrip:
 *#curpos: id of the terminal whose buffer is needed to be cleared
 * */
void getTerminalCurPos(int * curpos);

/*
 * getTerminalCurPos
 *
 *descrip:
 *#curpos: id of the terminal whose buffer is needed to be cleared
 * */
int mkfifo( int * fd);

/*
 * getTerminalCurPos
 *
 *descrip:
 *#curpos: id of the terminal whose buffer is needed to be cleared
 * */
void rmfifo( int * fd);

/*
 * getTerminalCurPos
 *
 *descrip:
 *#curpos: id of the terminal whose buffer is needed to be cleared
 * */
void semget(int * key, int initvalue, int * status);

/*
 * getTerminalCurPos
 *
 *descrip:
 *#curpos: id of the terminal whose buffer is needed to be cleared
 * */
void up(int key);

/*
 * down
 *
 *descrip:
 *#curpos: id of the terminal whose buffer is needed to be cleared
 * */
void down(int key);

/*
 * semrm
 *
 *descrip:
 *#curpos: id of the terminal whose buffer is needed to be cleared
 * */
void semrm(int key); /*implement*/

/*
 * mkDir
 *
 *descrip:
 *#curpos: id of the terminal whose buffer is needed to be cleared
 * */
void mkDir(char * newName);

/*
 * touch
 *
 *descrip:
 *#curpos: id of the terminal whose buffer is needed to be cleared
 * */
void touch( char * filename );

/*
 * cat
 *
 *descrip:
 *#curpos: id of the terminal whose buffer is needed to be cleared
 * */
void cat( char * filename );

/*
 * cd
 *
 *descrip:
 *#curpos: id of the terminal whose buffer is needed to be cleared
 * */
void cd(char * path);

/*
 * link
 *
 *descrip:
 *#curpos: id of the terminal whose buffer is needed to be cleared
 * */
void link(char * path1, char * path2);


