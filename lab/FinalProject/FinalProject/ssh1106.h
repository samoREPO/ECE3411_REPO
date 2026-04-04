/*
 * ssh1106.h
 *
 * Created: 4/1/2026 7:47:30 PM
 *  Author: samou
 */ 


#ifndef SSH1106_H_
#define SSH1106_H_

#include <stdint.h>

#define SH1106_ADDR 0x3C   // common, sometimes 0x3D depending on module

void SH1106_Command(uint8_t cmd);
void SH1106_CommandList(const uint8_t *cmds, uint8_t len);
void SH1106_Data(uint8_t data);
void SH1106_DataBuffer(uint8_t *data, uint16_t len);

void SH1106_Init(void);
void SH1106_SetPageColumn(uint8_t page, uint8_t col);
void SH1106_Clear(void);
void SH1106_Fill(void);

#endif /* SSH1106_H_ */
