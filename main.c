/*
 * cors.c
 *
 * Created: 14.10.2019 21:08:46
 * Author : serge
 */ 

#ifndef F_CPU
#define F_CPU 1000000UL // 1 MHz clock speed
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include "lcd.h"

#define  PORT_THERMO_CS           PORTB
#define  DDR_THERMO_CS            DDRB
#define  BIT_THERMO_CS            2
#define  MASK_THERMO_CS           (1<<BIT_THERMO_CS)

#define  PORT_SPI                 PORTB
#define  DDR_SPI                  DDRB
#define  BIT_SPI_SCK              7
#define  MASK_SPI_SCK             (1<<BIT_SPI_SCK)
#define  BIT_SPI_SS				  5
#define  MASK_SPI_SS              (1<<BIT_SPI_SS)
#define  BIT_SPI_MISO             6
#define  MASK_SPI_MISO            (1<<BIT_SPI_MISO)


int test[4];

/*
 *  ThermoInit      set up hardware for using the MAX31855
 *
 *  This routine configures the SPI as a master for exchanging
 *  data with the MAX31855 thermocouple converter.  All pins
 *  and registers for accessing the various port lines are
 *  defined at the top of this code as named literals.
 */
static void  ThermoInit(void)
{
    PORT_THERMO_CS |= MASK_THERMO_CS;        // start with CS high
    DDR_THERMO_CS |= MASK_THERMO_CS;         // now make that line an output

    PORT_SPI |= MASK_SPI_SS;                 // SS* is not used but must be driven high
    DDR_SPI |= MASK_SPI_SS;                  // SS* is not used but must be driven high
    PORT_SPI &= ~MASK_SPI_SCK;               // drive SCK low
    DDR_SPI |= MASK_SPI_SCK;                 // now make SCK an output

    SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR0) | (1<<SPR1) | (1<<CPHA);
                                             // enable SPI as master, slowest clock,
                                             // data active on trailing edge of SCK
}


/*
 *  ThermoReadRaw      return 32-bit raw value from MAX31855
 *
 *  This routine uses a four-byte SPI exchange to collect a
 *  raw reading from the MAX31855 thermocouple converter.  That
 *  value is returned unprocessed to the calling routine.
 *
 *  Note that this routine does NO processing.  It does not
 *  check for error flags or reasonable data ranges.
 */
static int32_t  ThermoReadRaw(void)
{
    int32_t                      d;
    unsigned char                n;
	int buf;
	
    PORT_THERMO_CS &= ~MASK_THERMO_CS;    // pull thermo CS low
    d = 0;                                // start with nothing
    for (n=3; n!=0xff; n--)
    {
        SPDR = 0;                         // send a null byte
        while ((SPSR & (1<<SPIF)) == 0)  ;    // wait until transfer ends
		buf = SPDR;
		test[n] = buf;
        d = (d<<8) + buf;                // add next byte, starting with MSB
    }
    PORT_THERMO_CS |= MASK_THERMO_CS;     // done, pull CS high

/*
 *                             Test cases
 *
 *  Uncomment one of the following lines of code to return known values
 *  for later processing.
 *
 *  Test values are derived from information in Maxim's MAX31855 data sheet,
 *  page 10 (19-5793 Rev 2, 2/12).
 */
//  d = 0x01900000;            // thermocouple = +25C, reference = 0C, no faults
//  d = 0xfff00000;            // thermocouple = -1C, reference = 0C, no faults
//  d = 0xf0600000;            // thermocouple = -250C, reference = 0C, no faults
//  d = 0x00010001;            // thermocouple = N/A, reference = N/A, open fault
//  d = 0x00010002;            // thermocouple = N/A, reference = N/A, short to GND
//  d = 0x00010004;            // thermocouple = N/A, refernece = N/A, short to VCC

    return  d;
}


/*
 *  ThermoReadC      return thermocouple temperature in degrees C
 *
 *  This routine takes a raw reading from the thermocouple converter
 *  and translates that value into a temperature in degrees C.  That
 *  value is returned to the calling routine as an integer value,
 *  rounded.
 *
 *  The thermocouple value is stored in bits 31-18 as a signed 14-bit
 *  value, where the LSB represents 0.25 degC.  To convert to an
 *  integer value with no intermediate float operations, this code
 *  shifts the value 20 places right, rather than 18, effectively
 *  dividing the raw value by 4 and scaling it to unit degrees.
 *
 *  Note that this routine does NOT check the error flags in the
 *  raw value.  This would be a nice thing to add later, when I've
 *  figured out how I want to propagate the error conditions...
 */
static int  ThermoReadC(void)
{
    char                        neg;
    int32_t                     d;

    neg = 0;                // assume a positive raw value
    d = ThermoReadRaw();        // get a raw value
    d = ((d >> 18) & 0x3fff);   // leave only thermocouple value in d
    if (d & 0x2000)             // if thermocouple reading is negative...
    {
        d = -d & 0x3fff;        // always work with positive values
        neg = 1;             // but note original value was negative
    }
    d = d + 2;                  // round up by 0.5 degC (2 LSBs)
    d = d >> 2;                 // now convert from 0.25 degC units to degC
    if (neg==1)  d = -d;           // convert to negative if needed
    return  d;                  // return as integer
}

int numKey[3][4]=	{{1, 2, 3},
					{4, 5, 6},
					{7, 8, 9},
					{10, 0, 11}};
char portState[4]= {0xF7,0xEF,0xDF,0xBF};
char inputState[3]={0x01,0x02,0x04};

static int readIntFromKeyboard()
{
	 for(int i=0; i<4; i++)
	 {
		 PORTA=portState[i];
		 for(int j=0; j<3; j++)
		 {
			 //_delay_ms(100);
			 if(((PINA & inputState[j])==0))
			 {
				 while((PINA & inputState[j])!=inputState[j]){};
				 return numKey[i][j];
			 }
		 }
	}
	return 0xFF;
}

int main(void)
{
    /* Replace with your application code */
		
		// Port D initialization
		// Func7=Out Func6=Out Func5=Out Func4=Out Func3=In Func2=In Func1=In Func0=In
		// State7=1 State6=1 State5=1 State4=1 State3=P State2=P State1=P State0=P
		PORTA=0xFF;
		DDRA=0xF8;
		
		
		 DDRC = 0xFF; //Nakes PORTC as Output
		   //_delay_ms(100);
		   ThermoInit();
		   
		   lcdInit();
		   lcdClear();
		   lcdSetDisplay(LCD_DISPLAY_ON);
		   lcdSetCursor(LCD_CURSOR_OFF);
		   
		   
		   //strcpy(text, "  Hello World!  ");
		   
		 while(1) //infinite loop
		 {
			 int TemperatureC = readIntFromKeyboard();
			 //TemperatureC = 10;
			 char text[17];
			 if(TemperatureC != 0xFF)
			 {
				 
			 
			 lcdClear();
			 lcdGotoXY(0, 0);
			 sprintf( text, "%d", TemperatureC );
			 
			 lcdPuts(text);
			 }
			 _delay_ms(100);
			 /*PORTC = 0xFF; //Turns ON All LEDs
			 _delay_ms(1000); //1 second delay
			 PORTC= 0x00; //Turns OFF All LEDs
			 _delay_ms(1000); //1 second delay*/
		 }
    
}

