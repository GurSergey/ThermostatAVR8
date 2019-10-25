#ifndef KEYBOARD3x4_H_
#define KEYBOARD3x4_H_
#include <avr/io.h>

#define KEYBOARD_SIZE_X		3
#define KEYBOARD_SIZE_Y		4
#define DEFAULT_RETURN 		0xFF
#define PORT_OF_KEYBOARD_STATE	PORTA
#define PORT_OF_KEYBOARD_INPUT	PINA
#define PORT_OF_KEYBOARD_DIRECTION DDRA


int readIntFromKeyboard();
void initKeyabord();

#endif