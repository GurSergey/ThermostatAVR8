#include "MAX31855.h"

void ThermoInit(void)
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

int32_t ThermoReadRaw(void)
{
    int32_t                      d;
    unsigned char                n;
	int buf;
	
    PORT_THERMO_CS &= ~MASK_THERMO_CS;    // pull thermo CS low
    d = 0;                                // start with nothing
    for (n=3; n!=0xff; n--)
    {
        SPDR = 0;                         // send a null byte
        while ((SPSR & (1<<SPIF)) == 0);    // wait until transfer ends
		buf = SPDR;
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

int ThermoReadC(void)
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