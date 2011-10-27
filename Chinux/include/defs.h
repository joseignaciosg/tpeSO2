/***************************************************
 Defs.h

 ****************************************************/

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
 * Calls (for int_80 function)
 */
#define WRITE 4
#define READ  5
#define ERASE 6

#define TRUE 1
#define FALSE 0

enum  state{ RUNNING = 0, READY, BLOCKED};
typedef enum state process_state;

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

#define ACS_CODE        (ACS_PRESENT | ACS_CSEG | ACS_READ) //el segmento de c�digo esta en memoria, y puede ser s�lo leido
#define ACS_DATA        (ACS_PRESENT | ACS_DSEG | ACS_WRITE)// el segemento de c�digo esta en memoria y puede ser escrito
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
	byte base_m, access, //contiene ubicación, longitud y derechos
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
	char name[20];
	char password[20];
}user;


#endif

