/*
 * DFR_LCD1602_LIB.c
 *
 * Created: 4/6/2026 1:42:02 PM
 *  Author: samou
 */ 

// LCD interface for AIP31068 LCD controllers
// By John Chandy, 2024

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <util/twi.h>

#include "DFR_LCD1602_LIB.h"
#include "I2C.h"

#define LCD_I2C_ADDR 0x3E
#define LCD_BACKLIGHT_I2C_ADDR 0x6B

void LCDdataWrite(uint8_t data)
{
	TWI_Host_Write_16bits(LCD_I2C_ADDR, 0x4000 | data);
}

static void LCDcommandWrite(uint8_t cm)
{
	TWI_Host_Write_16bits(LCD_I2C_ADDR, 0x0000 | cm);
}

void LCDinitialize(void)
{
	_delay_ms(50);	// wait 50ms for LCD display controller to initialize

	TWI_Host_Initialize(false);

	LCDcommandWrite(0x28);	// enable 2-row
	_delay_us(39);
	LCDvisible();		// turn display on (by default at power on, it is off)
	LCDclr();			// clear the display

	TWI_Host_Write_16bits(LCD_BACKLIGHT_I2C_ADDR, 0x2F00); /* Reset backlight */
	TWI_Host_Write_16bits(LCD_BACKLIGHT_I2C_ADDR, 0x0020); /* Enable backlight */
	TWI_Host_Write_16bits(LCD_BACKLIGHT_I2C_ADDR, 0x04FF); /* Max out backlight PWM on blue channel */
	TWI_Host_Write_16bits(LCD_BACKLIGHT_I2C_ADDR, 0x0700); /* update pwm register value */
}

void LCDclr(void) //Clears LCD
{
	LCDcommandWrite(0x01);
	_delay_us(1530);
}

void LCDhome(void) //LCD cursor home
{
	LCDcommandWrite(0x02);
	_delay_us(1530);
}

void LCDstring(char* data)	//Outputs string to LCD
{
	TWI_Address(LCD_I2C_ADDR, TW_WRITE);
	TWI_Transmit_Data(0x40);
	// print data
	for(int i=0; data[i]; i++) {
		TWI_Transmit_Data((uint8_t)data[i]);
	}
	TWI_Stop();
}

void LCDgotoXY(uint8_t x, uint8_t y) //Cursor to X Y position
{
	register uint8_t position = (y << 6) + x;
	LCDcommandWrite(0x80 + position);
	_delay_us(39);
}

//Copies string from flash memory to LCD at x y position
//const uint8_t welcomeln1[] PROGMEM="AVR LCD DEMO\0";
//LCDcopyStringFromFlash(welcomeln1, 3, 1);
void LCDcopyStringFromFlash(const uint8_t *FlashLoc, uint8_t x, uint8_t y)
{
	uint8_t i = 0;
	LCDgotoXY(x,y);
	TWI_Address(LCD_I2C_ADDR, TW_WRITE);
	TWI_Transmit_Data(0x40);
	while(1) {
		uint8_t data = (uint8_t)pgm_read_byte(&FlashLoc[i]);
		if (data == 0) break;
		TWI_Transmit_Data(data);
		i++;
	}
	TWI_Stop();
}

void LCDshiftLeft(uint8_t n)	//Scrol n of characters Right
{
	for (uint8_t i=0;i<n;i++) {
		LCDcommandWrite(0x18);
		_delay_us(39);
	}
}

void LCDshiftRight(uint8_t n)	//Scrol n of characters Left
{
	for (uint8_t i=0;i<n;i++) {
		LCDcommandWrite(0x1C);
		_delay_us(39);
	}
}

void LCDcursorOnBlink(void)	//displays LCD blinking cursor
{
	LCDcommandWrite(0x0F);
	_delay_us(39);
}

void LCDcursorOn(void)	//turns OFF cursor
{
	LCDcommandWrite(0x0E);
	_delay_us(39);
}

void LCDcursorOff(void)	//turns OFF cursor
{
	LCDcommandWrite(0x0C);
	_delay_us(39);
}

void LCDblank(void)		//blanks LCD
{
	LCDcommandWrite(0x08);
	_delay_us(39);
}

void LCDvisible(void)		//Shows LCD
{
	LCDcommandWrite(0x0C);
	_delay_us(100);
}

void LCDcursorLeft(uint8_t n)	//Moves cursor by n positions left
{
	for (uint8_t i=0;i<n;i++) {
		LCDcommandWrite(0x10);
		_delay_us(39);
	}
}

void LCDcursorRight(uint8_t n)	//Moves cursor by n positions left
{
	for (uint8_t i=0;i<n;i++) {
		LCDcommandWrite(0x14);
		_delay_us(39);
	}
}
