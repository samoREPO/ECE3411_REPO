/*
 * lcd_lib.c
 *
 * Created: 3/8/2026 10:44:56 AM
 *  Author: samou
 */ 

#include "lcd_lib.h"
#include "I2C_lib.h"
#include <util/delay.h>

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
	LCDclr();
	LCDcommandWrite(0x30); // DL=1:8-bit bus mode, N=0:1-line display mode, F=0: 5x8 font type // TODO: write masks for these commands
	LCDcommandWrite(DISPLAY_CTRL); // Display ctrl: D=0:Display Off, C=0:Cursor off, B=0: blink off
	LCDcommandWrite(0x06); // TODO write masks
}
		
void LCDdataWrite(uint8_t data)
{
	LCDwriteByte(DATA_CTRL_BYTE, data);
}

void LCDclr(void) //Clears LCD
{
	LCDcommandWrite(CLR_DISPLAY);
	_delay_ms(2);
}

void LCDhome(void){
	LCDcommandWrite(0x02);
	_delay_ms(2);
}
void LCDstring(char* str) //Outputs string to LCD
{
	
}
void LCDgotoXY(uint8_t x, uint8_t y) //Cursor to X Y position
{
	
}
void LCDcopyStringFromFlash(const uint8_t* str, uint8_t x, uint8_t y) //copies flash string to LCD at x,y
{
	
}
void LCDshiftRight(uint8_t n) //shift by n characters Right
{
	
}
void LCDshiftLeft(uint8_t n) //shift by n characters Left
{
	
}
void LCDcursorOn(void) //cursor ON
{
	LCDcommandWrite(0x0E);
	// TODO: Add delay
}
void LCDcursorOnBlink(void)	//Blinking cursor ON
{
	LCDcommandWrite(0x0F);
	//TODO: Add delay
}
void LCDcursorOff(void)	//Both underline and blinking cursor OFF
{
	LCDcommandWrite(0x0C);
	//TODO: Add delay
}
void LCDblank(void) //LCD blank but not cleared
{
	LCDcommandWrite(0x80);
	//TODO: Add delay
}

void LCDvisible(void) //LCD visible
{
	LCDcommandWrite(0x0C);
	//TODO: Add delay
}
void LCDcursorLeft(uint8_t n) //shift cursor left by n
{
	
}
void LCDcursorRight(uint8_t n) //shift cursor right by n	
{
	
}			
