/*
 * lcd_lib.h
 *
 * LCD interface for AIP31068 LCD controllers
 * By John Chandy, 2024
 */

#ifndef LCD_LIB_H_
#define LCD_LIB_H_

#include <inttypes.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

// LCD ADDRESS DEFINITIONS
#define LCD_ADDR 0x3E

// BACKLIGHT ADDRESS DEFINITIONS
#define RGB_ADDR 0x6B
#define RESET_REG 0x2F
#define OUT1 0x04
#define OUT2 0x05
#define OUT3 0x06
#define SHUTDOWN_REG 0x00

// LCD CTRL BYTE DEFINITIONS
#define CMD_CTRL_BYTE 0x00
#define DATA_CTRL_BYTE 0x40

void LCDinitialize(void);                        // Initializes LCD
void LCDdataWrite(uint8_t);
void LCDclr(void);                              // Clears LCD
void LCDhome(void);                             // LCD cursor home
void LCDstring(char*);                          // Outputs string to LCD
void LCDgotoXY(uint8_t, uint8_t);               // Cursor to X Y position
void LCDcopyStringFromFlash(const uint8_t*, uint8_t, uint8_t);
void LCDshiftRight(uint8_t);                    // shift by n characters Right
void LCDshiftLeft(uint8_t);                     // shift by n characters Left
void LCDcursorOn(void);                         // cursor ON
void LCDcursorOnBlink(void);                    // Blinking cursor ON
void LCDcursorOff(void);                        // Both underline and blinking cursor OFF
void LCDblank(void);                            // LCD blank but not cleared
void LCDvisible(void);                          // LCD visible
void LCDcursorLeft(uint8_t);                    // shift cursor left by n
void LCDcursorRight(uint8_t);                   // shift cursor right by n

#endif /* LCD_LIB_H_ */