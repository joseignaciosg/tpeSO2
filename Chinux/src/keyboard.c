/********************************** 
 *
 *  Keyboard.c
 *  	Galindo, Jose Ignacio
 *  	Homovc, Federico
 *		ITBA 2011
 *
 ***********************************/

/***	Project Includes	***/
#include "../include/defs.h"
#include "../include/stdio.h"
#include "../include/kc.h"


char buffcopy[BUFFER_SIZE];

/*variables used in shell*/
int keypressed = FALSE;
int caps_lock = -1;
unsigned shift_state = FALSE;
unsigned ctrl_state = FALSE;
unsigned alt_state = FALSE;
unsigned supr_state = FALSE;
unsigned up_arrow_state = FALSE;
unsigned down_arrow_state = FALSE;
extern TTY terminals[4];
extern int currentTTY;
extern char * vidmem;
extern int scan_test;
extern int currentTTY;
extern int password;
extern int usrLoged;
extern int CurrentPID;

void memcpy(char* a, char* b, int len);

void readKeyboard(char* buffer, size_t count) {
	int k = 0;
	int flag = TRUE;
	if (terminals[currentTTY].buffer.size != 0) {
		if (terminals[currentTTY].buffer.size >= BUFFER_SIZE || terminals[currentTTY].buffer.size == 0) {
			flag = FALSE;
		}
		while (k < count) {
			if (((terminals[currentTTY].buffer.actual_char == terminals[currentTTY].buffer.first_char - 1)
					|| (terminals[currentTTY].buffer.actual_char == BUFFER_SIZE - 1
							&& terminals[currentTTY].buffer.first_char == 0)) && flag) {
				break;
			}
			flag = TRUE;
			buffer[k++] = terminals[currentTTY].buffer.array[terminals[currentTTY].buffer.first_char++ % BUFFER_SIZE];
		}
	}
	return;
}

void clearBuffcopy(void) {
	int i;
	for (i = 0; i < BUFFER_SIZE; i++) {
		buffcopy[i] = 0;
	}
	return;
}

void deleteCharFromBuff() {
	if (terminals[currentTTY].buffer.size != 0) {
		if(!password)
			clearc(' ');
		if (terminals[currentTTY].buffer.actual_char == 0) {
			terminals[currentTTY].buffer.actual_char = BUFFER_SIZE - 1;
		} else {
			terminals[currentTTY].buffer.actual_char--;
		}
		terminals[currentTTY].buffer.size--;
	}

	return;
}

int addCharToBuff(char c) {
	if (terminals[currentTTY].buffer.size < BUFFER_SIZE) {
		terminals[currentTTY].buffer.array[++terminals[currentTTY].buffer.actual_char % BUFFER_SIZE] = c;
		terminals[currentTTY].buffer.size++;
		terminals[currentTTY].buffer.actual_char = terminals[currentTTY].buffer.actual_char % BUFFER_SIZE;
		return TRUE;
	}

	return FALSE;
}

void int_09() {
	char c;
	int i;
	unsigned char new_scan_code = _inport(0x60);
	static const unsigned char
		shiftTable[][83] = { { 0/*Esc*/, '1', '2', '3', '4', '5', '6', '7',
				'8', '9', '0', '-', '=', '\b', 0/*Tab*/, 'q', 'w', 'e',
				'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
				0/*ctrl*/, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',
				';', '\'', '`', 0/*Lshift*/, '\\', 'z', 'x', 'c', 'v', 'b',
				'n', 'm', ',', '.', '/', 0/*Rshift*/, 0, 0/*alt*/, ' ',
				0/*caps*/, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0 },

		{ 0/*Esc*/, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_',
				'+', '\b', 0, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O',
				'P', '{', '}', '\n', 0/*ctrl*/, 'A', 'S', 'D', 'F', 'G',
				'H', 'J', 'K', 'L', ':', '"', '~', 0/*Lshift*/, '|', 'Z',
				'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0/*Rshift*/,
				0, 0/*alt*/, ' ', 0/*caps*/, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };

	switch (new_scan_code) {
	case 0x2a: //for shifts
		shift_state = TRUE;
		break;
	case 0x36:
		shift_state = TRUE;
		break;
	case 0xaa:
		shift_state = FALSE;
		break;
	case 0xb6:
		shift_state = FALSE;
		break;
	case 0x1d: //for ctrl
		ctrl_state = TRUE;
		break;
	case 0x9d:
		ctrl_state = FALSE;
		break;
	case 0x38: //for alt
		alt_state = TRUE;
		break;
	case 0xb8:
		alt_state = FALSE;
		break;
	case 0x3A:
		caps_lock = -caps_lock;
		break;
	case 0x53: //for supr
		supr_state = TRUE;
		break;
	case 0xD3:
		supr_state = FALSE;
		break;
	case 0x48: //up arrow
		up_arrow_state = TRUE;
		break;
	case 0x50: //down arrow
		down_arrow_state = TRUE;
		break;

	case 0x0e: //backspace key pressed
		if (terminals[currentTTY].buffer.size != 0) {
			deleteCharFromBuff();
			moveCursor();
		}
		break;
	case 0x1c: //enter pressed
		if(terminals[currentTTY].buffer.size == 0 && usrLoged)
		{
			putc('\n');
			printShellLine();
		}
		awake_process(terminals[currentTTY].PID);
		break;

	default:

		if( new_scan_code == 0x2E && ctrl_state)
		{
			kill(CurrentPID);
			break;
		}

		//if(new_scan_code>=0x3B && new_scan_code<=0x3E && alt_state) // entre F1 y F4
		if(new_scan_code >= 0x2 && new_scan_code <= 0x5 && alt_state && usrLoged)
		{
			int nextTTY;
			PROCESS * proc;
			//nextTTY = new_scan_code - 0x3B;
			nextTTY = new_scan_code - 0x2;
			block_process(terminals[currentTTY].PID);
			memcpy(terminals[currentTTY].terminal, vidmem, 80 * 2 * 25);
			memcpy(vidmem, terminals[nextTTY].terminal, 80 * 2 * 25);
			currentTTY = nextTTY;
			moveCursor();
			proc = GetProcessByPID(terminals[currentTTY].PID);
			if(terminals[currentTTY].buffer.size == 0 && proc->waitingPid == 0)
				awake_process(terminals[currentTTY].PID);
			break;
		}

		/* Ignore the break code */
		if (shiftTable[0][new_scan_code] == 0 && new_scan_code != 0x39)
			break;

		if (new_scan_code >= 0x02 && new_scan_code <= 0x53) {

			if ((shift_state && caps_lock == -1) ||	(!shift_state && caps_lock == 1))
				c = shiftTable[1][new_scan_code - 1];
			else
				c = shiftTable[0][new_scan_code - 1];
			keypressed = addCharToBuff(c);
		}
		break;
	}

	if (keypressed) {
		if(!password)
		{
			putc(terminals[currentTTY].buffer.array[terminals[currentTTY].buffer.actual_char]);
			moveCursor();
		}
		keypressed = FALSE;
	} else if (up_arrow_state && scan_test == NOTSCAN) {
		showLastCommand();
	} else if (down_arrow_state && scan_test == NOTSCAN) {
		showPreviousCommand();
	} else if (shift_state && ctrl_state && supr_state) {
			reboot();
	}
	return;
}

void memcpy(char* a, char* b, int len)
{
	int i;
	for(i = 0; i < len; i++)
		a[i] = b[i];
	return;
}
