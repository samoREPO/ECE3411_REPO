/*
 * lcd_lib.c
 *
 * Created: 3/8/2026 10:44:56 AM
 *  Author: samou
 */ 

#include "lcd_lib.h"
#include "I2C_lib.h"
#include <util/delay.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/twi.h>

static void BacklightwriteByte(uint8_t address, uint8_t value)
{
	TWI_Address(RGB_ADDR, TW_WRITE);
	TWI_Transmit_Data(address);
	TWI_Transmit_Data(value);
	TWI_Stop();
	
}

static void BacklightReset(void)
{
	BacklightwriteByte(0x2F, 0x00);
} 

static void setPWMRegister(uint8_t address, uint8_t value)
{
	BacklightwriteByte(address, value);
	BacklightwriteByte(0x07, 0x00);
}

static void enableShutdownRegister(void)
{
	BacklightwriteByte(0x00, 0x20);
}

static void LCDwriteByte(uint8_t control, uint8_t value)
{
	TWI_Address(LCD_ADDR, TW_WRITE);
	TWI_Transmit_Data(control);
	TWI_Transmit_Data(value);
	TWI_Stop();
}

static void LCDcommandWrite(uint8_t cmd)
{
	LCDwriteByte(CMD_CTRL_BYTE, cmd);
}

void LCDinitialize(void)
{
	BacklightReset();
	enableShutdownRegister();
	setPWMRegister(0x04, 0xFF);
	
	LCDclr();
	LCDvisible();
	
	LCDcommandWrite(0x38);
	_delay_us(39);
}
		
void LCDdataWrite(uint8_t data)
{
	LCDwriteByte(DATA_CTRL_BYTE, data);
}

void LCDclr(void) //Clears LCD
{
	LCDcommandWrite(0x01);
	_delay_us(1530);
}

void LCDhome(void){
	LCDcommandWrite(0x02);
	_delay_ms(2);
}
void LCDstring(char* str) //Outputs string to LCD
{
	while(*str != '\0')
	{
		LCDdataWrite(*str); str++;
	}
}
void LCDgotoXY(uint8_t x, uint8_t y) //Cursor to X Y position
{
	uint8_t destination = x;
	if(y==1) destination += 0x40;

	LCDcommandWrite(0x80 | destination);
	_delay_us(39);
}

void LCDcopyStringFromFlash(const uint8_t* str, uint8_t x, uint8_t y) //copies flash string to LCD at x,y
{
	LCDgotoXY(x,y);
	uint8_t data;
	do 
	{
		data = pgm_read_byte(str);
		if (data != '\0')
		{
			LCDdataWrite(data);
			str++;
		}
	} while (data != '\0');

}

void LCDshiftRight(uint8_t n) //shift by n characters Right
{
	for(;n>0;n--){
		LCDcommandWrite(0x1C);
		_delay_us(39);
	}
}
void LCDshiftLeft(uint8_t n) //shift by n characters Left
{
	for(;n>0;n--){
		LCDcommandWrite(0x18);
		_delay_us(39);
	}
}
void LCDcursorOn(void) //cursor ON
{
	LCDcommandWrite(0x0E);
	_delay_us(39);
}
void LCDcursorOnBlink(void)	//Blinking cursor ON
{
	LCDcommandWrite(0x0F);
	_delay_us(39);
}
void LCDcursorOff(void)	//Both underline and blinking cursor OFF
{
	LCDcommandWrite(0x0C);
	_delay_us(39);
}
void LCDblank(void) //LCD blank but not cleared
{
	LCDcommandWrite(0x08);
	_delay_us(39);
}

void LCDvisible(void) //LCD visible
{
	LCDcommandWrite(0x0C);
	_delay_us(39);
}
void LCDcursorLeft(uint8_t n) //shift cursor left by n
{
	for(;n>0;n--){
		LCDcommandWrite(0x10);
		_delay_us(39);
	}
}
void LCDcursorRight(uint8_t n) //shift cursor right by n	
{
	for(;n>0;n--){
		LCDcommandWrite(0x14);
		_delay_us(39);
	}	
}			
