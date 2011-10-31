/********************************** 
*
*  defs.h
*  	Galindo, Jose Ignacio
*  	Homovc, Federico
*  	Loreti, Nicolas
*		ITBA 2011
*
***********************************/

#ifndef _defs_
#define _defs_

#define byte unsigned char
#define word short int
#define dword int

/*
 * File descriptors
 */
#define STDOUT 1 //corresponds to the screen
#define STDIN  2 //corresponds to the keyboard
//maybe more descriptors could be added later

/*
 * System Calls (for int_80 function)
 */
#define WRITE 4
#define READ  5
#define ERASE 6

/*
 * System Calls (for int_79 function)
 */
#define KILL 7
#define BLOCK 8
#define CREATE 9
#define CLEAR_TERM 10
#define WAIT_PID 11
#define TERM_SIZE 12
#define TERM_CURPOS 13
#define CURR_TTY 14
#define MK_FIFO 15
#define SEM_GET 16
#define SEM_UP 17
#define SEM_DOWN 18
#define MK_DIR 19
#define LS_COM 20
#define RM_COM 21
#define TOUCH_COM 22
#define CAT_COM 23
#define LINK_COM 24
#define CD_COM 25
#define CREAT_COM 26
#define RM_FIFO 27
#define SEM_RM 28


#define TRUE 1
#define FALSE 0

enum  state{ RUNNING = 0, READY, BLOCKED};
typedef enum state process_state;

enum  groups{ ADMIN = 0, USR};
typedef enum groups groupID;


typedef int size_t;
typedef short int ssize_t;

/* Flags para derechos de acceso de los segmentos */
#define ACS_PRESENT     0x80            /* segmento presente en memoria */
#define ACS_CSEG        0x18            /* segmento de codigo */
#define ACS_DSEG        0x10            /* segmento de datos */
#define ACS_READ        0x02            /* segmento de lectura */
#define ACS_WRITE       0x02            /* segmento de escritura */
#define ACS_IDT         ACS_DSEG
#define ACS_INT_386 	0x0E		/* Interrupt GATE 32 bits */
#define ACS_INT         ( ACS_PRESENT | ACS_INT_386 )

#define ACS_CODE        (ACS_PRESENT | ACS_CSEG | ACS_READ) //el segmento de cï¿œdigo esta en memoria, y puede ser sï¿œlo leido
#define ACS_DATA        (ACS_PRESENT | ACS_DSEG | ACS_WRITE)// el segemento de cï¿œdigo esta en memoria y puede ser escrito
#define ACS_STACK       (ACS_PRESENT | ACS_DSEG | ACS_WRITE)//el segmento de pila esta en memoria , y puede ser escrito
#pragma pack (1) 		/* Alinear las siguiente estructuras a 1 byte */

#define BUFFER_SIZE 256

#define MAX_NUM 25		/*Maxima cantidad de digitos de un int*/

#define MAX_PRIORITY 4
#define PRIORITY_RATIO 2
#define TIMESLOT 3

#define NULL 0

/* Descriptor de segmento *///GDT
typedef struct {
	word limit, base_l;
	byte base_m, access, //contiene ubicaciÃ³n, longitud y derechos
	attribs, //ver pagina 35 libro brey
	base_h;
} DESCR_SEG;

/* Descriptor de interrupcion */
typedef struct {
	word offset_l, selector;
	byte cero, access;
	word offset_h;
} DESCR_INT;

/* IDTR  */
typedef struct {
	word limit;
	dword base;
} IDTR;

/*
 * Main structure for handling the keyboard
 */
typedef struct {
	char array[BUFFER_SIZE];
	unsigned int actual_char;
	unsigned int first_char;
	unsigned int size;
} KEY_BUFFER;

typedef struct
{
	int EDI, ESI, EBP, ESP, EBX, EDX, ECX, EAX, EIP, CS, EFLAGS;
	void * retaddr;
	int argc;
	char ** argv;
} STACK_FRAME;

typedef struct PROCESS
{
	process_state state;
	int pid;
	char * name;
	int priority;
	int tty;
	int foreground;
	int parent;
	int ESP;
	int stackstart;
	int stacksize;
	int waitingPid;
	int sleep;
	int acum;

} PROCESS;


typedef struct
{
	char terminal[80 * 25 * 2];
	KEY_BUFFER buffer;
	int curpos;
	int PID;
}TTY;

typedef struct processNode * processList;

typedef struct{
	PROCESS * process;
	processList next;
}processNode;

typedef struct{
	char name[256];
	char password[256];
	int usrID;
	groupID group;
}user;

typedef struct{
	int pid;
	int cpu;
}processCPU;

typedef struct{
	char* name;
	int (*process)(int,char**);
	int tty;
	int argc;
	char** argv;
	int stacklength;
	int priority;
	int isFront;
}createProcessParam;

typedef struct{
	char * name;/*TODO test*/
	size_t fd1;/*fifo«s file descriptor 1*/
	size_t fd2;/*fifo«s file descriptor 2*/
}fifoStruct;

typedef struct{
	int key;
	int value;
	int blocked_proc_pid;
	int status;
}semItem;

typedef struct{
	int fd;
	char * file;
	int curr_size;
	int sem_key;
}my_fdItem;


typedef struct{
	char * path1;
	char * path2;
}link_struct;

typedef struct{
	char * filename;
	int mode;
}creat_param;

#endif

