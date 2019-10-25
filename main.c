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

ISR(INT2_vect)
{
	debounce();
	PORTC = 0xFF;
	PORTC = 0x00;
}

void inputMode()
{
}

void setMode()
{

}

#define PWM_PRESCALER 1024
#define PWM_FREQUENCY 10
#define PWM_TOP_TIMER_VALUE F_CPU/1024/PWM_FREQUENCY
#define PWM_MODE_3 0.9
#define PWM_MODE_2 0.5 
#define PWM_MODE_1 0.25
#define PWM_MODE_0 0

void pwmInit()
{
		TCCR1A = ((1 << COM1A1) | (1 << WGM11)); // clear OC1A on match + WGM mode 14
		TCCR1B = ((1 << WGM12) | (1 << WGM13) | (1 << CS12) | (1 << CS10)); // WGM mode 14 (Fast PWM)
		// and 1024x prescaler
		ICR1  = PWM_TOP_TIMER_VALUE;  //set ICR1 to produce 50Hz frequency
		OCR1A = PWM_MODE_1*PWM_TOP_TIMER_VALUE;  
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
		 
		   
		   //strcpy(text, "  Hello World!  ");
		 sei();
		 while(1) //infinite loop
		 {
			 /*int TemperatureC = readIntFromKeyboard();
			 char text[17];

			 lcdClear();
			 lcdGotoXY(0, 0);
			 sprintf( text, "%d", TemperatureC );
			 
			 lcdPuts(text);*/
			 _delay_ms(100);
		 }
    
}

