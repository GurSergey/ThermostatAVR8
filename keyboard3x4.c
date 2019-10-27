#include "KEYBOARD3X4.h"

int numKey[4][3]=	{{1, 2, 3},
					{4, 5, 6},
					{7, 8, 9},
					{10, 0, 11}};
char portState[4]= {0xF7,0xEF,0xDF,0xBF};
char inputState[3]={0x01,0x02,0x04};


int readIntFromKeyboard()
{
	PORT_OF_KEYBOARD_STATE = 0x00;
	 for(int i=0; i<KEYBOARD_SIZE_Y; i++)
	 {
		 PORT_OF_KEYBOARD_STATE = portState[i];
		 for(int j=0; j<KEYBOARD_SIZE_X; j++)
		 {
			 if(((PORT_OF_KEYBOARD_INPUT & inputState[j]) == 0))
			 {
				 while((PORT_OF_KEYBOARD_INPUT & inputState[j])!=inputState[j]){};
				 return numKey[i][j];
			 }
		 }
	}
	return DEFAULT_RETURN_KEYBOARD;
}

void initKeyabord()
{
	PORT_OF_KEYBOARD_STATE=0xFF;
	PORT_OF_KEYBOARD_DIRECTION=0xF8;	
}

