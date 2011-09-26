|/**********************************
*
*  Keyboard.c
*  	Galindo, Jose Ignacio
*  	Homovc, Federico
*	Reznik, Luciana
*		ITBA 2011
*
***********************************/

/***	Proyect Includes	***/
#include "../include/defs.h"
#include "../include/stdio.h"


/***	Module Defines	***/
#define GETTICKTIMES 5

KEY_BUFFER keybuffer;
char buffcopy[BUFFER_SIZE];

/*variables used in shell*/
int keypressed = FALSE;
int enterpressed = FALSE;
int caps_lock = -1;
unsigned shift_state = FALSE;
unsigned ctrl_state = FALSE;
unsigned alt_state = FALSE;
unsigned supr_state = FALSE;
unsigned up_arrow_state = FALSE;
unsigned down_arrow_state = FALSE;
extern int curpos;



void
initializeKeyBuffer(){
	keybuffer.actual_char = BUFFER_SIZE-1;
	keybuffer.first_char = 0;
	keybuffer.size = 0;
	
	return;
}

void
readKeyboard(char* buffer, size_t count)
{
	int k = 0;
	int flag = TRUE;
	if ( keybuffer.size != 0 ){
		if ( keybuffer.size >= BUFFER_SIZE || keybuffer.size == 0){
			flag = FALSE;
		}
		while (k < count){
			if ((( keybuffer.actual_char == keybuffer.first_char-1) ||
				(keybuffer.actual_char == BUFFER_SIZE-1 && keybuffer.first_char == 0) ) && flag){
				break;
			}
			flag = TRUE;
			buffer[k++] = keybuffer.array[ keybuffer.first_char++ % BUFFER_SIZE];
		}
	}
	return;
}


void 
clearBuffcopy(void) {
	int i;
	for (i = 0; i <BUFFER_SIZE; i++) {
		buffcopy[i] = 0;
	}
	return;
}

void
deleteCharFromBuff(){
	if ( keybuffer.size != 0 ) {
		clearc(' ');
		if ( keybuffer.actual_char == 0 ){
			keybuffer.actual_char = BUFFER_SIZE -1;
		}else{
			keybuffer.actual_char--;
		}
		keybuffer.size--;
	}

	return;
}

int
addCharToBuff(char c){
	if(keybuffer.size < BUFFER_SIZE){
		keybuffer.array[++keybuffer.actual_char % BUFFER_SIZE] = c;
		keybuffer.size++;
		keybuffer.actual_char = keybuffer.actual_char % BUFFER_SIZE;
		return TRUE;
	}

	return FALSE;
}

void int_09() {
	unsigned char new_scan_code = _inport(0x60);
	char c;
	static const unsigned char tablaShift[][83] = { 
			{ 0/*Esc*/, '1', '2', '3', '4', '5',
			'6', '7', '8', '9', '0', '-', '=', '\b',
			0/*Tab*/, 'q', 'w', 'e', 'r', 't',
			'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
			0/*ctrl*/, 'a', 's', 'd', 'f', 'g', 'h', 'j',
			'k', 'l', ';', '\'', '`', 0/*Lshift*/, '\\',
			'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.',
			'/', 0/*Rshift*/, 0, 0/*alt*/, ' ', 0/*caps*/,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0 },

			{ 0/*Esc*/, '!', '@', '#', '$', '%', '^', '&',
			'*', '(', ')', '_', '+', '\b', 0, 'Q', 'W', 'E',
			'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
			0/*ctrl*/, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K',
			'L', ':', '"', '~', 0/*Lshift*/, '|', 'Z', 'X', 'C',
			'V', 'B', 'N', 'M', '<', '>', '?', 0/*Rshift*/, 0,
			0/*alt*/, ' ', 0/*caps*/, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };

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
	case 0x1c:
		enterpressed=TRUE;
		break;
	case 0x48: //up arrow
		up_arrow_state=TRUE;
		break;
	case 0x50: //down arrow
		down_arrow_state=TRUE;
		break;

	case 0x0e: //backspace key pressed
		if ( keybuffer.size != 0 ) {
			deleteCharFromBuff();
			moveCursor();
		}
		break;

	default:
		/* Ignore the break code */
		if(tablaShift[0][new_scan_code] == 0 && new_scan_code != 0x39) //0x39 = space makecode
			break;

		if (new_scan_code >= 0x02 && new_scan_code <= 0x53 ) {

			if ( (shift_state && caps_lock == -1) || (!shift_state && caps_lock == 1))
				c = tablaShift[1][new_scan_code - 1];
			else
				c = tablaShift[0][new_scan_code - 1];
			keypressed = addCharToBuff(c);
		}
		break;
	}
	return;
}
