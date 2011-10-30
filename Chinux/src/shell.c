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
#include "../include/process.h"
#include "../include/fs.h"
#include "../include/atadisk.h"
#include "../include/apps.h"
#include "../include/kernel.h"

/*#include "../include/kernel.h"*/

/***	Module Defines	***/
#define STO_MAX  100

char storedComm[STO_MAX][BUFFER_SIZE + 1];
int arrow_pressed = FALSE;
int sto_i = STO_MAX - 1;
int sel_com = STO_MAX - 1;
int usrLoged = 0;
int usrName = 0;
int password = 0;
processCPU top100[100];
int logoutPID = -1;
user usr;

extern int last100[100];
extern int currentProcessTTY;
extern int currentTTY;
extern int CurrentPID;
extern int logPID;
extern user admin;

void logout(int argc, char * argv[]);




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
	printf("printftest:    shows a test for printf function");
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
	if (sel_com > 0 && usrLoged) {
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
		getTerminalSize(&size);
		if (size!=0) {
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
	if (sel_com < sto_i + 1 && usrLoged) {
		if (arrow_pressed) {
			aux = sel_com;
			size = str_len(storedComm[aux]);
			for (i = 0; i < size; i++) {
				deleteCharFromBuff();
			}
			getTerminalSize(&size);
			if (size!=0) {
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
	int buffsize;
	getTerminalSize(&buffsize);
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

void splitbuffer(void)
{
	int i = 0, j = 0, k = 0, size;
	getTerminalSize(&size);
	while(j < size)
	{
		buffcopyparsed[i][k] = buffcopy[j];
		k++;
		if(buffcopy[j] == ' ' && i < 9)
		{
			buffcopyparsed[i][k] = '\0';
			i++; k = 0;
		}
		j++;
	}
	buffcopyparsed[i][k] = '\0';
	return ;
}


int
parseBuffer() {
	int invalidcom = FALSE;
	int cleared_screen = FALSE;
	int isFront = 1, pid, i,size,curpos;
	getTerminalSize(&size);

	scanf("%s", buffcopy);
	
	splitbuffer();

	if(!usrLoged && usrName)
	{
		strcopy(usr.name, buffcopy, size);
		clearBuffcopy();
		clearTerminalBuffer(currentTTY);
		return;
	}

	if(!usrLoged && password)
	{
		strcopy(usr.password, buffcopy, size);
		clearBuffcopy();
		if(strcmp(usr.name, admin.name) && strcmp(usr.password, admin.password))
			usrLoged = 1;
		else
			printf("\nUser name or password incorrect. Please try again.");
		clearTerminalBuffer(currentTTY);

		return;
	}

	saveCommand();

	if(buffcopy[size - 1] == '&' && buffcopy[size - 2] == ' ')
	{
		isFront = 0;
		buffcopy[size - 2] = 0;
	}

	if (strcmp("clear", buffcopy)) {
		k_clear_screen();
		cleared_screen = TRUE;
		isFront = 0;
	} else if (strcmp("printftest", buffcopy)) {
		putc('\n');
		printfTest();
		isFront = 0;
	} else if (strcmp("getCPUSpeed", buffcopy)) {
		putc('\n');
		printf("CPU Speed: %ld  MHz", getCPUSpeed());
	} else if (strcmp("help", buffcopy)) {
		putc('\n');
		showHelp();
		isFront = 0;
	}else if(strcmp("prueba", buffcopy)){
		//putc('\n');
		pid = CreateProcessAt("Prueba", (int(*)(int, char**))prueba, currentProcessTTY, 0, (char**)0, 0x400, 2, isFront);
	}else if(strcmp("prueba2", buffcopy)){
		//putc('\n');
		pid = CreateProcessAt("Prueba2", (int(*)(int, char**))prueba2, currentProcessTTY, 0, (char**)0, 0x400, 2, isFront);
	}else if(strcmp("prioridad0", buffcopy)){
		pid = CreateProcessAt("prioridad0", (int(*)(int, char**))prioridad, currentProcessTTY, 0, (char**)0, 0x400, 0, isFront);
	}else if(strcmp("prioridad1", buffcopy)){
		pid = CreateProcessAt("prioridad1", (int(*)(int, char**))prioridad, currentProcessTTY, 0, (char**)0, 0x400, 1, isFront);
	}else if(strcmp("prioridad2", buffcopy)){
		pid = CreateProcessAt("prioridad2", (int(*)(int, char**))prioridad, currentProcessTTY, 0, (char**)0, 0x400, 2, isFront);
	}else if(strcmp("prioridad3", buffcopy)){
		pid = CreateProcessAt("prioridad3", (int(*)(int, char**))prioridad, currentProcessTTY, 0, (char**)0, 0x400, 3, isFront);
	}else if(strcmp("prioridad4", buffcopy)){
		pid = CreateProcessAt("prioridad4", (int(*)(int, char**))prioridad, currentProcessTTY, 0, (char**)0, 0x400, 4, isFront);
	}else if(strcmp("logout", buffcopy)){
		int aux = currentProcessTTY;
		clearBuffcopy();
		for(i = 0; i < 4; i++)
		{
			clearTerminalBuffer(i);
			currentProcessTTY = i;
			k_clear_screen();
			cleared_screen = TRUE;
		}
		currentProcessTTY = aux;
		logoutPID = pid = CreateProcessAt("logout", (int(*)(int, char**))logout, currentProcessTTY, 0, (char**)0, 0x400, 4, isFront);
	}else if(strcmp("top", buffcopy)){
		pid = CreateProcessAt("Top", (int(*)(int, char**))top, currentProcessTTY, 0, (char**)0, 0x400, 2, isFront);
	}else if(strcmp("kill ", buffcopyparsed[0])){
		int pid;
		scanfi(&pid, &buffcopyparsed[1][0]);
		kill(pid);
		isFront = 0;
	}else if(strcmp("createusr ", buffcopyparsed[0])){
		strcopy(usr.name, buffcopyparsed[1], str_len(buffcopyparsed[1]) );
		strcopy(usr.password, buffcopyparsed[2], str_len(buffcopyparsed[2]) );
		printf("name:%s password:%s\n", usr.name, usr.password);
		//pid = CreateProcessAt("Top", (int(*)(int, char**))top, currentProcessTTY, 0, (char**)0, 0x400, 2, isFront);
	}else if(strcmp("mkdir ", buffcopyparsed[0])){
		makeDir(buffcopyparsed[1]);
		isFront = 0;
	}else if(strcmp("ls ", buffcopyparsed[0])){
		ls(buffcopyparsed[1]);
		isFront = 0;
	}else if(strcmp("rm ", buffcopyparsed[0])){
		rmDir(buffcopyparsed[1]);
		isFront = 0;
	}else if(strcmp("touch", buffcopyparsed[0])){
		touch(buffcopyparsed[1]);
		isFront = 0;
	}else if(strcmp("cat ", buffcopyparsed[0])){
		cat(buffcopyparsed[1]);
		isFront = 0;
	}else if(strcmp("ln ", buffcopyparsed[0])){
		link(buffcopyparsed[1],buffcopyparsed[2]);
		isFront = 0;
	}else if(strcmp("cd ", buffcopyparsed[0])){
		cd(buffcopyparsed[1]);/*rmDir*/
		isFront = 0;
	}else {

		invalidcom = TRUE;
		isFront = 0;
	}

	getTerminalCurPos(&curpos);
	if (curpos > 80 * 24 * 2) {
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
		} else{
			putc('\n');
		}
	}

	clearBuffcopy();

	if(isFront)
		return pid;
	return isFront;
}



void
shell(int argc, char * argv[]) {
	int command, pid;
	if(currentProcessTTY != currentTTY)
		block_process(CurrentPID);
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
			/*TODO testing*/
			clearTerminalBuffer(currentTTY);
		}
		moveCursor();
		_Sti();
	}
	return;
}


int
read_command()
{
	int size;
	getTerminalSize(&size);
	if(size == 0)
		return -1;
	return 0;
}
