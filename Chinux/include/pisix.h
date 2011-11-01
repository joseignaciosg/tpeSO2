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
 * getTerminalSize
 *
 *descrip: gets the terminal buffer size
 *#size: terminal buffer size
 * */
void getTerminalSize(int * size);

/*
 * getTerminalCurPos
 *
 *descrip: gets the terminal current position
 *#curpos: current position of terminal
 * */
void getTerminalCurPos(int * curpos);

/*
 * mkfifo
 *
 *descrip: creates a fifo
 *#fd: vector of two ints for the fifo fd
 * */
int mkfifo( int * fd);

/*
 * rmfifo
 *
 *descrip:removes fifo
 *#fd: vector of two ints for the fifo fd
 * */
void rmfifo( int * fd);

/*
 * semget
 *
 *descrip: gets an semaphore
 *#key: semaphore key
 *#initvalue: initial value
 *#status: out status
 * */
void semget(int * key, int initvalue, int * status);

/*
 * up
 *
 *descrip: increases semaphore
 *#key: semaphore key
 * */
void up(int key);

/*
 * down
 *
 *descrip: decreases semaphore
 *#key: semaphore key
 * */
void down(int key);

/*
 * semrm
 *
 *descrip:removes semaphore
 *#key: semaphore key
 * */
void semrm(int key);

/*
 * mkDir
 *
 *descrip: creates a new directory
 *#newName: directory name
 * */
void mkDir(char * newName);

/*
 * touch
 *
 *descrip: creates an empty file
 *#filename: name of the file
 * */
void touch( char * filename );

/*
 * cat
 *
 *descrip: prints the content of a file
 *#filename:name of the file
 * */
void cat( char * filename );

/*
 * cd
 *
 *descrip: changes current dir
 *#path: path to go
 * */
void cd(char * path);

/*
 * link
 *
 *descrip: links a file
 *#path1: path of the file
 *#path2: path of the link
 * */
void link(char * path1, char * path2);


