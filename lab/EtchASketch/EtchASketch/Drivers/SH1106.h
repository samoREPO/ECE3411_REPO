/*
 * SH1106.h
 *
 * Created: 4/4/2026 11:25:17 AM
 *  Author: samou
 */ 

#ifndef SH1106_H_
#define SH1106_H_

#include <stdint.h>

#define SH1106_I2C_ADDR  0x3C

#define SH1106_WIDTH     128
#define SH1106_HEIGHT    64
#define SH1106_PAGES     8
#define SH1106_BUF_SIZE  (SH1106_WIDTH * SH1106_PAGES)

extern uint8_t sh1106_buffer[SH1106_BUF_SIZE];

void SH1106_Init(void);
void SH1106_UpdateScreen(void);

void SH1106_ClearBuffer(void);
void SH1106_FillBuffer(void);

void SH1106_SetPixel(uint8_t x, uint8_t y);
void SH1106_ClearPixel(uint8_t x, uint8_t y);
void SH1106_WritePixel(uint8_t x, uint8_t y, uint8_t color);

void SH1106_InvertDisplay(uint8_t enable);

#endif /* SH1106_H_ */