/********************************** 
*
*  shell.c
*  	Galindo, Jose Ignacio
*  	Homovc, Federico
*	Reznik, Luciana
*		ITBA 2011
*
***********************************/

/***	Proyect Includes	***/
#include "../include/defs.h"
#include "../include/shell.h"
#include "../include/stdio.h"
#include "../include/tests.h"

/***	Module Defines	***/
#define STO_MAX  100

int scan_test = NOTSCAN;
extern unsigned int curpos;
char storedComm[STO_MAX][BUFFER_SIZE + 1];
int sto_i = STO_MAX-1;
int sel_com = STO_MAX-1;
char arrow_pressed = FALSE;

char * splash_screen[25] = {
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
};

void
printShellLine()
{
	int i;
	for (i = 0; i < SHELL_LINE_LENGTH; i++) {
		putc(SHELL_LINE[i]);
	}
	return;
}

void
showHelp()
{
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
        printf("%s",splash_screen[i]);
    }
	return;
}


void
wait(double sec){
	tickswait = 0;
	while ( (tickswait * 0.055 ) <= sec )
		;
	return;
}

void
showLastCommand(){
	int aux;
	int size;
	int i, k;
	if ( sel_com > 0){
		if ( arrow_pressed ){
			if (sel_com == STO_MAX-1)
				aux = 0;
			else
				aux = sel_com;
			size = str_len(storedComm[aux]);
			for (i = 0; i<size; i++ ){
				deleteCharFromBuff();
			}
		}
		if( keybuffer.array[0] ){
			size = keybuffer.size;
			for(i = 0; i < size; i++){
				deleteCharFromBuff();
			}
		}
		sel_com--;
		printf("%s", storedComm[sel_com]);
		for (k = 0; k < str_len(storedComm[sel_com]);k++){
			addCharToBuff(storedComm[sel_com][k]);
		}
	}
	up_arrow_state = FALSE;
	arrow_pressed = TRUE;

	return;
}

void
showPreviousCommand(){
	int aux;
	int size;
	int i, k;
	if ( sel_com < sto_i+1){
		if ( arrow_pressed ){
			aux = sel_com;
			size = str_len(storedComm[aux]);
			for (i = 0; i<size; i++ ){
				deleteCharFromBuff();
			}
		}
		sel_com++;
		if ( sel_com >= 0 && sel_com < STO_MAX && storedComm[sel_com][0] ){
			printf("%s", storedComm[sel_com]);
			for (k = 0; k < str_len(storedComm[sel_com]); k++){
				addCharToBuff(storedComm[sel_com][k]);
			}
		}
	}
	down_arrow_state = FALSE;
	arrow_pressed = TRUE;

	return;
}

void
saveCommand(){
	int buffsize = keybuffer.size;
	if ( buffcopy[0]){
		if ( sto_i == STO_MAX-1 )
			sto_i = 0;
		else
			sto_i++;
		sel_com = sto_i+1;
		strcopy(storedComm[sto_i],buffcopy, buffsize);
	}

	return;
}

void
parseBuffer()
{
	int invalidcom = FALSE;
	int cleared_screen = FALSE;
	scan_test = NOTSCAN;

	scanf("%s",buffcopy);

	saveCommand();

	if( strcmp("clear", buffcopy)){
		k_clear_screen();
		cleared_screen = TRUE;
	}else if( strcmp("scanint",buffcopy)){
		scan_test = SCANINT;
		putc('\n');
		printf("Type an integer: ");
	}else if( strcmp("scandouble",buffcopy)){
		scan_test = SCANDOUBLE;
		putc('\n');
		printf("Type a double: ");
	}else if( strcmp("scanstring",buffcopy)){
		scan_test = SCANSTRING;
		putc('\n');
		printf("Type a string: ");
	}else if(strcmp("printftest",buffcopy)){
		putc('\n');
		printfTest();
	}else if(strcmp("getCPUSpeed",buffcopy)){
		putc('\n');
		printf("CPU Speed: %ld  Hz", getCPUSpeed());
	}else if(strcmp("help",buffcopy)){
		putc('\n');
		showHelp();
		clearBuffcopy();
	}else{
		invalidcom = TRUE;
	}

	if(curpos>80*24*2 && scan_test == NOTSCAN){
		scrolldown();
		if ( invalidcom && buffcopy[0]){
			invalidcom = FALSE;
			printf("Invalid command: %s\n", buffcopy);
		}
	}
	else if(!cleared_screen){
		if ( invalidcom && buffcopy[0]){
			invalidcom = FALSE;
			putc('\n');
			printf("Invalid command: %s\n", buffcopy);
		}else if(scan_test == NOTSCAN){
			putc('\n');
		}
	}

	clearBuffcopy();
	
	return;
}


void
shell(int argc, char* argv[])
{
	//_Sti();
	printShellLine();
	moveCursor();
	while(TRUE)
       	{
		if ( keypressed ){
			putc(keybuffer.array[keybuffer.actual_char]);
			keypressed = FALSE;
		}else if( up_arrow_state && scan_test == NOTSCAN){
			showLastCommand();
		}else if(down_arrow_state && scan_test == NOTSCAN){
			showPreviousCommand();
		}/*else if (enterpressed ){

			switch( scan_test ){
				 case NOTSCAN:
					 parseBuffer();
					 arrow_pressed = FALSE;
					 if ( !scan_test )
						 printShellLine();
					 break;
				 case SCANSTRING:
					 putc('\n');
					 scanStringTest();
					 scan_test=NOTSCAN;
					 clearBuffcopy();
					 printShellLine();
					 break;
				 case SCANINT:
					 putc('\n');
					 scanIntTest();
					 scan_test=NOTSCAN;
					 clearBuffcopy();
					 printShellLine();
					 break;
				 case SCANDOUBLE:
					 putc('\n');
					 scanDoubleTest();
					 scan_test=NOTSCAN;
					 clearBuffcopy();
					 printShellLine();
					 break;
	    		}
	    		keybuffer.first_char = keybuffer.actual_char + 1 % BUFFER_SIZE;
	    		keybuffer.size = 0;
	    		enterpressed = FALSE;
	   	}*/else if ( alt_state && ctrl_state && supr_state){
	   		reboot();
	   	}
	moveCursor();
	}
	return;
}


