/*
 * cors.c
 *
 * Created: 14.10.2019 21:08:46
 * Author : serge
 */ 

#ifndef F_CPU
#define F_CPU 1000000UL // 1 MHz clock speed
#endif

/* External Interrupt Request 2 */
#define INT2_vect			_VECTOR(18)
#define SIG_INTERRUPT2			_VECTOR(18)


#define INPUT_TEMPERATURE_MODE 1
#define TEMPERATURE_SET_MODE 2
#define MIN_TEMPERATURE_C 25
#define MAX_TEMPERATURE_C 125
#define DEBOUNCE_MS 10

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <avr/interrupt.h>
#include "lcd.h"
#include "keyboard3x4.h"
#include "max31855.h"

int test[4];

int currentTemperature;
int necessaryTemperature;

void debounce()
{
	_delay_ms(DEBOUNCE_MS);
}

#define COUNT_OF_PIN_SHIFT 16
#define SH_CP_PIN 0
#define DS_PIN 1
#define ST_CP_PIN 2
#define PORT_SHIFT_REGISTER PORTC

void switchOnN(int N)
{
	PORT_SHIFT_REGISTER &= ~(1<<SH_CP_PIN | 1<<DS_PIN | 1<<ST_CP_PIN);
	for(int i = 0; i < COUNT_OF_PIN_SHIFT; i++)
	{
		PORT_SHIFT_REGISTER ^= (1<<SH_CP_PIN );
		PORT_SHIFT_REGISTER ^= (1<<SH_CP_PIN );
	}
	for(int i = 0; i < N; i++)
	{
		PORT_SHIFT_REGISTER ^= (1<<SH_CP_PIN | 1<<DS_PIN);
		PORT_SHIFT_REGISTER ^= (1<<SH_CP_PIN | 1<<DS_PIN);;
	}
	PORT_SHIFT_REGISTER ^=  (1<<ST_CP_PIN);
	PORT_SHIFT_REGISTER ^=  (1<<ST_CP_PIN);
}

#define MODE_INPUT 0
#define MODE_HEAT 1
#define MAX_COUNT_MODE 2
int currentMode = MODE_INPUT;
char changedModeFlag = 1;

ISR(INT2_vect)
{
	debounce();
	if(currentMode==MODE_HEAT)
	{
		pwmOff();
		switchOnN(0);
	}
	
	if(currentMode==MAX_COUNT_MODE-1)
	{
		currentMode = MODE_INPUT;
	}
	else
	{
		currentMode++;
		
	}
	changedModeFlag = 1;
	
	
	//_delay_ms(1000);
	
}


#define DEFAULT_INPUT_VALUE_TEMPERATURE 0
#define MAX_INPUT_VALUE 125
#define MIN_INPUT_VALUE 0
int currentInputTemp = DEFAULT_INPUT_VALUE_TEMPERATURE ;

void inputMode()
{
	char text[17];
	int valueFromKeyboard = readIntFromKeyboard();
	char isChange = 0;
	if((valueFromKeyboard != DEFAULT_RETURN_KEYBOARD) && (valueFromKeyboard != ERASE_KEY))
	{
		int buffer = currentInputTemp;
		currentInputTemp =currentInputTemp*10+valueFromKeyboard;
		if(currentInputTemp>MAX_INPUT_VALUE)
			currentInputTemp = buffer;
		else
			isChange = 1;	
	}
	if(valueFromKeyboard == ERASE_KEY)
	{
		currentInputTemp = currentInputTemp/10;
		isChange = 1;
	}
	if((isChange == 1)||(changedModeFlag==1))
	{
		lcdClear();
		lcdGotoXY(0, 0);
		sprintf( text, "Input T: %d", currentInputTemp );
		lcdPuts(text);
		changedModeFlag = 0;
	}
}


#define PWM_PRESCALER 1024
#define PWM_FREQUENCY 10
#define PWM_TOP_TIMER_VALUE F_CPU/1024/PWM_FREQUENCY
#define PWM_MODE_3 0.9
#define PWM_MODE_2 0.5 
#define PWM_MODE_1 0.25
#define PWM_MODE_0 0
#define COUNT_LEDS 10

#define DEFAULT_TEMP 0
int lastTemperature = DEFAULT_TEMP;
void heatMode()
{
	char text[17];
	int temperature = ThermoReadC();
	if((lastTemperature!=temperature) ||(changedModeFlag==1))
	{
		changedModeFlag = 0;
		lcdClear();
		lcdGotoXY(0, 0);
		sprintf( text, "Now:%d Need:%d", temperature, currentInputTemp );
		lcdPuts(text);
		lastTemperature = temperature;
		int countLedToGlow = COUNT_LEDS - (currentInputTemp - temperature) / (((float)MAX_INPUT_VALUE-MIN_INPUT_VALUE)/COUNT_LEDS);
		if(countLedToGlow>COUNT_LEDS)
		{
			countLedToGlow = COUNT_LEDS;
		}
		if(countLedToGlow<0)
		{
			countLedToGlow = 0;
		}
		
		switchOnN(countLedToGlow);
		if(currentInputTemp - temperature >= (MAX_INPUT_VALUE-MIN_INPUT_VALUE)/2)
			pwmModeSelect(PWM_MODE_3);
		else
		if(currentInputTemp - temperature >= (MAX_INPUT_VALUE-MIN_INPUT_VALUE)/4)
			pwmModeSelect(PWM_MODE_2);
		else
		if(temperature>=currentInputTemp)
		{
			pwmModeSelect(PWM_MODE_0);
			pwmOff();
		}
		else
		if(currentInputTemp - temperature < (MAX_INPUT_VALUE-MIN_INPUT_VALUE)/4 )
			pwmModeSelect(PWM_MODE_1);
		
		
	}
	
	
}

void pwmInit()
{
		TCCR1A = ((0 << COM1A1) | (1 << WGM11)); // clear OC1A on match + WGM mode 14
		TCCR1B = ((1 << WGM12) | (1 << WGM13) | (1 << CS12) | (1 << CS10)); // WGM mode 14 (Fast PWM)
		// and 1024x prescaler
		ICR1  = PWM_TOP_TIMER_VALUE; 
		OCR1A = PWM_MODE_0*PWM_TOP_TIMER_VALUE;  
}

void pwmModeSelect(float mode)
{
		TCCR1A = ((1 << COM1A1) | (1 << WGM11));
		OCR1A = mode*PWM_TOP_TIMER_VALUE;
}

void pwmOff()
{
	TCCR1A = ((0 << COM1A1) | (1 << WGM11));
}



int main(void)
{
    /* Replace with your application code */
		
		initKeyabord();
		DDRD = 0xFF;
		pwmInit();
		
		//включим прерывания INT2 по восходящему 
		MCUCSR |= (1<<ISC2);

		//разрешим внешние прерывания 
		GICR |= (1<<INT2);
		
		
		 DDRC = 0xFF; //Nakes PORTC as Output
		 
	
			

			
		   ThermoInit();
		   
		   lcdInit();
		   lcdClear();
		   lcdSetDisplay(LCD_DISPLAY_ON);
		   lcdSetCursor(LCD_CURSOR_OFF);
		 
		   char text[17];
		   //strcpy(text, "  Hello World!  ");
		 sei();
		 while(1) //infinite loop
		 {
			 
			 switch(currentMode)
			 {
				case MODE_INPUT:
					inputMode();
					break;
				case MODE_HEAT:
					heatMode();
					break;	
			 }
			 
			 /*int TemperatureC = readIntFromKeyboard();
			 

			 lcdClear();
			 lcdGotoXY(0, 0);
			 sprintf( text, "%d", TemperatureC );
			 
			 lcdPuts(text);*/
			 _delay_ms(100);
		 }
    
}

