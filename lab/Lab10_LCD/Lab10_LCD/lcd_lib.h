// LCD interface for AIP31068 LCD controllers
// By John Chandy, 2024

#include <inttypes.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

// LCD ADDRESS DEFINITIONS
#define LCD_ADDR 0x7C // TODO: confirm this is the right position of address. does it need to be shifted?
#define RGB_ADDR 0xC0 // TODO: confirm this is the right position of address. does it need to be shifted?

// LCD CTRL BYTE DEFINITIONS
#define CMD_CTRL_BYTE 0x00
#define DATA_CTRL_BYTE 0x40

// LCD INTRUCTION DEFINITIONS
// -- Clear Display
#define CLR_DISPLAY 0x01
// -- Return Home -- return cursor to its original site and return display to its original status, if shifted
#define RETURN_HOME 0x02
// -- Entry Mode Set -- Set the moving direction of cursor or display
// -- -- Inc/Dec cursor (DDRAM address)
#define CURSOR_LEFT 0x04
#define CURSOR_RIGHT 0x06
// -- -- SHR/SHL Display -- SH bit high turns on display shift
#define DISPLAY_LEFT 0x07
#define DISPLAY_RIGHT 0x05

// -- Display control
#define DISPLAY_CTRL 0x08
// -- -- Display ON/OFF Control bit
#define DISPLAY_ON_bm 0x04
// -- -- Cursor ON/OFF Control bit
#define CURSOR_ON_bm 0x02
// -- -- Cursor Blink ON/OFF Control bit
#define CURSOR_BLINK_ON_bm 0x01

// TODO check the difference between this and earlier function
// -- Cursor or Display Shift -- Shifting of right/left cursor position or display without writing or reading of display data
// -- -- S/C = 0, R/L = 0 : Shift cursor left, AC decremented

// -- 6 -- Function Set
#define FUNCTION_SET 0x20

// -- 7 -- Set CGRAM Address
// -- -- CGRAM set mask
#define SET_GCRAM_bm 0x40

// -- 8 -- Set DDRAM Address
// -- -- DDRAM set mask
#define SET_DDRAM_bm 0x80

// -- 9 -- Read Busy Flag & Address

void LCDinitialize(void);		//Initializes LCD
void LCDdataWrite(uint8_t);
void LCDclr(void);				//Clears LCD
void LCDhome(void);				//LCD cursor home
void LCDstring(char*);			//Outputs string to LCD
void LCDgotoXY(uint8_t, uint8_t);	//Cursor to X Y position
void LCDcopyStringFromFlash(const uint8_t*, uint8_t, uint8_t); //copies flash string to LCD at x,y
void LCDshiftRight(uint8_t);	//shift by n characters Right
void LCDshiftLeft(uint8_t);	    //shift by n characters Left
void LCDcursorOn(void);			//cursor ON
void LCDcursorOnBlink(void);	//Blinking cursor ON
void LCDcursorOff(void);		//Both underline and blinking cursor OFF
void LCDblank(void);			//LCD blank but not cleared
void LCDvisible(void);			//LCD visible
void LCDcursorLeft(uint8_t);	//shift cursor left by n
void LCDcursorRight(uint8_t);	//shift cursor right by n
