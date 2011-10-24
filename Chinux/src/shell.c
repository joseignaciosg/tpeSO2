/**********************************
 *
 *  shell.c
 *  	Galindo, Jose Ignacio
 *  	Homovc, Federico
 *		ITBA 2011
 *
 ***********************************/

/***	Project Includes	***/
#include "../include/defs.h"
#include "../include/shell.h"
#include "../include/stdio.h"
#include "../include/tests.h"
#include "../include/utils.h"
#include "../include/keyboard.h"
#include "../include/video.h"
#include "../include/kc.h"

/***	Module Defines	***/
#define STO_MAX  100

int scan_test = NOTSCAN;
char storedComm[STO_MAX][BUFFER_SIZE + 1];
int sto_i = STO_MAX - 1;
int sel_com = STO_MAX - 1;
char arrow_pressed = FALSE;
int usrLoged = 0;
int usrName = 0;
int password = 0;
user usr;

extern int currentTTY;
extern TTY terminals[4];
extern int CurrentPID;
extern user admin;
extern int currentProcessTTY;
static int read_command();
void prueba(int argc, char * argv[]);
void prueba2(int argc, char * argv[]);
void top(int argc, char * argv[]);
void logUser(void);
void waitpid(int pid);
int top100[100] = {0};
extern int last100[100];

char
* splash_screen[25] = {
	"                                                                               ",
	"                                                                               ",
	"                                                                               ",
	"                                                                               ",
	"                                                                               ",
	"                                                                               ",
	"                                                                               ",
	"            ..%%%%...%%..%%..%%%%%%..%%..%%..%%..%%..%%..%%.                   ",
	"             .%%..%%..%%..%%....%%....%%%.%%..%%..%%...%%%%..                  ",
	"              .%%......%%%%%%....%%....%%.%%%..%%..%%....%%...                 ",
	"               .%%..%%..%%..%%....%%....%%..%%..%%..%%...%%%%..                ",
	"                ..%%%%...%%..%%..%%%%%%..%%..%%...%%%%...%%..%%.               ",
	"                                                                               ",
	"                                                                               ",
	"                                                                               ",
	"                                                                               ",
	"                                                                               ",
	"                                                                               ",
	"                                                                               ",
	"                                                                               ",
	"                                                                               ",
	"                                                                               ",
	"                                                                               ",
	"                                                                               "};


void
printShellLine() {
	printf("%s", SHELL_LINE);
	return;
}


void
showHelp() {
	printf("help:          displays this message \n");
	printf("clear:         clears screen \n");
	printf("getCPUSpeed:   returns the clock speed of the processor \n");
	printf("Ctrl+Alt+Supr: reboots the system \n");
	printf("printftest:    shows a test for printf function \n");
	printf("scanint:       scanf integer test function \n");
	printf("scandouble:    scanf double test function \n");
	printf("scanstring:    scanf string test function");
	return;
}

void
showSplashScreen() {
	int i;
	for (i = 0; i < 24; i++) {
		printf("%s", splash_screen[i]);
	}
	return;
}

void
wait(int sec) {
	tickswait = 0;
	while ((tickswait * 0.055) <= sec)
		;
	return;
}

void
showLastCommand() {
	int aux;
	int size;
	int i, k;
	if (sel_com > 0) {
		if (arrow_pressed) {
			if (sel_com == STO_MAX - 1)
				aux = 0;
			else
				aux = sel_com;
			size = str_len(storedComm[aux]);
			for (i = 0; i < size; i++) {
				deleteCharFromBuff();
			}
		}
		if (terminals[currentTTY].buffer.array[0]) {
			size = terminals[currentTTY].buffer.size;
			for (i = 0; i < size; i++) {
				deleteCharFromBuff();
			}
		}
		sel_com--;
		printf("%s", storedComm[sel_com]);
		for (k = 0; k < str_len(storedComm[sel_com]); k++) {
			addCharToBuff(storedComm[sel_com][k]);
		}
	}
	up_arrow_state = FALSE;
	arrow_pressed = TRUE;
	moveCursor();

	return;
}

void
showPreviousCommand() {
	int aux;
	int size;
	int i, k;
	if (sel_com < sto_i + 1) {
		if (arrow_pressed) {
			aux = sel_com;
			size = str_len(storedComm[aux]);
			for (i = 0; i < size; i++) {
				deleteCharFromBuff();
			}
			if (terminals[currentTTY].buffer.array[0]) {
				size = terminals[currentTTY].buffer.size;
				for (i = 0; i < size; i++) {
					deleteCharFromBuff();
				}
			}
		}
		sel_com++;
		if (sel_com >= 0 && sel_com < STO_MAX && storedComm[sel_com][0]) {
			printf("%s", storedComm[sel_com]);
			for (k = 0; k < str_len(storedComm[sel_com]); k++) {
				addCharToBuff(storedComm[sel_com][k]);
			}
		}
	}
	down_arrow_state = FALSE;
	arrow_pressed = TRUE;
	moveCursor();

	return;
}

void
saveCommand() {
	int buffsize = terminals[currentTTY].buffer.size;
	if (buffcopy[0]) {
		if (sto_i == STO_MAX - 1)
			sto_i = 0;
		else
			sto_i++;
		sel_com = sto_i + 1;
		strcopy(storedComm[sto_i], buffcopy, buffsize);
	}

	return;
}

int
parseBuffer() {
	int invalidcom = FALSE;
	int cleared_screen = FALSE;
	int isFront = 1, pid;
	scan_test = NOTSCAN;

	scanf("%s", buffcopy);

	if(!usrLoged && usrName)
	{
		strcopy(usr.name, buffcopy, terminals[currentTTY].buffer.size);
		clearBuffcopy();
		terminals[currentTTY].buffer.first_char = terminals[currentTTY].buffer.actual_char + 1 % BUFFER_SIZE;
		terminals[currentTTY].buffer.size = 0;
		return;
	}

	if(!usrLoged && password)
	{
		strcopy(usr.password, buffcopy, terminals[currentTTY].buffer.size);
		clearBuffcopy();
		if(strcmp(usr.name, admin.name) && strcmp(usr.password, admin.password))
			usrLoged = 1;
		else
			printf("\nUser name or password incorrect. Please try again.");
		terminals[currentTTY].buffer.first_char = terminals[currentTTY].buffer.actual_char + 1 % BUFFER_SIZE;
		terminals[currentTTY].buffer.size = 0;		
		return;
	}

	saveCommand();

	if(buffcopy[terminals[currentTTY].buffer.size - 1] == '&' && buffcopy[terminals[currentTTY].buffer.size - 2] == ' ')
	{
		isFront = 0;
		buffcopy[terminals[currentTTY].buffer.size - 2] = 0;
	}

	if (strcmp("clear", buffcopy)) {
		k_clear_screen();
		cleared_screen = TRUE;
		isFront = 0;
	} else if (strcmp("scanint", buffcopy)) {
		scan_test = SCANINT;
		putc('\n');
		printf("Type an integer: ");
	} else if (strcmp("scandouble", buffcopy)) {
		scan_test = SCANDOUBLE;
		putc('\n');
		printf("Type a double: ");
	} else if (strcmp("scanstring", buffcopy)) {
		scan_test = SCANSTRING;
		putc('\n');
		printf("Type a string: ");
	} else if (strcmp("printftest", buffcopy)) {
		putc('\n');
		printfTest();
	} else if (strcmp("getCPUSpeed", buffcopy)) {
		putc('\n');
		printf("CPU Speed: %ld  MHz", getCPUSpeed());
	} else if (strcmp("help", buffcopy)) {
		putc('\n');
		showHelp();
	}else if(strcmp("prueba", buffcopy)){
		//putc('\n');
		pid = CreateProcessAt("Prueba", (int(*)(int, char**))prueba, currentProcessTTY, 0, (char**)0, 0x400, 2, isFront);
	}else if(strcmp("prueba2", buffcopy)){
		//putc('\n');
		pid = CreateProcessAt("Prueba2", (int(*)(int, char**))prueba2, currentProcessTTY, 0, (char**)0, 0x400, 2, isFront);
	}else if(strcmp("top", buffcopy)){
		pid = CreateProcessAt("Top", (int(*)(int, char**))top, currentProcessTTY, 0, (char**)0, 0x400, 2, isFront);
	}else if(buffcopy[0] == 'k' && buffcopy[1] == 'i' && buffcopy[2] == 'l' && buffcopy[3] == 'l' && buffcopy[4] == ' '){
		int pid;
		scanfi(&pid, &buffcopy[5]);
		kill(pid);
		isFront = 0;
	}else {
		invalidcom = TRUE;
		isFront = 0;
	}

	if (terminals[currentProcessTTY].curpos > 80 * 24 * 2 && scan_test == NOTSCAN) {
		scrolldown();
		if (invalidcom && buffcopy[0]) {
			invalidcom = FALSE;
			printf("Invalid command: %s\n", buffcopy);
		}
	} else if (!cleared_screen) {
		if (invalidcom && buffcopy[0]) {
			invalidcom = FALSE;
			putc('\n');
			printf("Invalid command: %s\n", buffcopy);
		} else if (scan_test == NOTSCAN) {
			putc('\n');
		}
	}

	clearBuffcopy();

	if(isFront)
		return pid;
	return isFront;
}

void prueba2(int argc, char * argv[])
{
	int i = 50000000;
	_Sti();
	//CreateProcessAt("Prueba", (int(*)(int, char**))prueba, currentTTY, 0, (char**)0, 0x400, 2, 1);
	//CreateProcessAt("Prueba", (int(*)(int, char**))prueba, currentTTY, 0, (char**)0, 0x400, 2, 1);
	//CreateProcessAt("Prueba", (int(*)(int, char**))prueba, currentTTY, 0, (char**)0, 0x400, 2, 1);
	printf("prueba2\n");
	while(i--)
		;

	return;
}

void prueba(int argc, char * argv[])
{
	int i = 50000000;
	_Sti();
	printf("prueba\n");
	while(i--)
		printf("prueba\n");
	return;
}


void top(int argc, char * argv[])
{
	int i, j;
	_Sti();
	while(TRUE)
	{
		for(i = 0; i < 100; i++)
			top100[i] = 0;
		for(i = 0; i < 100; i++)
			top100[last100[i]]++;
		printf("pid   %%cpu\n");
		for(i = 0; i < 100; i++)
			if(top100[i] != 0)
				printf("%d   %d\n", i, top100[i]);
		sleep(2);
	}
}

void
logUser(void)
{
	while(!usrLoged)
	{
		printf("username: ");
		moveCursor();
		usrName = 1;
		block_process(CurrentPID);
		parseBuffer();
		usrName = 0;
		printf("\n");
		printf("password: ");
		moveCursor();
		password = 1;
		block_process(CurrentPID);
		parseBuffer();
		password = 0;
		printf("\n");
	}
	return;
}

void
shell(int argc, char * argv[]) {
	int command, pid;

	if(currentTTY != argc)
		block_process(CurrentPID);
	logUser();
	printShellLine();
	moveCursor();
	while (TRUE) {

		_Cli();
		command = read_command();
		if(command == -1)
		{
			block_process(CurrentPID);
		}
		else {
			pid = parseBuffer();
			if(pid)
				waitpid(pid);
			printShellLine();
			terminals[currentTTY].buffer.first_char = terminals[currentTTY].buffer.actual_char + 1 % BUFFER_SIZE;
			terminals[currentTTY].buffer.size = 0;
		}
		moveCursor();
		_Sti();
	}
	return;
}

void waitpid(int pid)
{
	PROCESS* proc;
	proc = GetProcessByPID(CurrentPID);
	proc->waitingPid = pid;
	block_process(CurrentPID);
}

int read_command()
{
	if(terminals[currentTTY].buffer.size == 0)
		return -1;
	return 0;
}
