/*
 * ssh1106.c
 *
 * Created: 4/1/2026 7:47:43 PM
 *  Author: samou
 */ 

#define F_CPU 16000000UL

#include "ssh1106.h"
#include "I2C.h"
#include <util/delay.h>

#define SH1106_CONTROL_CMD   0x00
#define SH1106_CONTROL_DATA  0x40

void SH1106_Command(uint8_t cmd)
{
	uint8_t packet[2] = {SH1106_CONTROL_CMD, cmd};
	TWI_Host_Write_Bytes(SH1106_ADDR, packet, 2);
}

void SH1106_CommandList(const uint8_t *cmds, uint8_t len)
{
	// simplest implementation: one command transaction per command
	for (uint8_t i = 0; i < len; i++) {
		SH1106_Command(cmds[i]);
	}
}

void SH1106_Data(uint8_t data)
{
	uint8_t packet[2] = {SH1106_CONTROL_DATA, data};
	TWI_Host_Write_Bytes(SH1106_ADDR, packet, 2);
}

void SH1106_DataBuffer(uint8_t *data, uint16_t len)
{
	// simple but inefficient version: one byte at a time
	for (uint16_t i = 0; i < len; i++) {
		SH1106_Data(data[i]);
	}
}

void SH1106_SetPageColumn(uint8_t page, uint8_t col)
{
	SH1106_Command(0xB0 | (page & 0x07));             // page address
	SH1106_Command(0x00 | (col & 0x0F));              // lower column nibble
	SH1106_Command(0x10 | ((col >> 4) & 0x0F));       // upper column nibble
}

void SH1106_Init(void)
{
	_delay_ms(100);

	SH1106_Command(0xAE); // display off

	SH1106_Command(0xD5); // clock divide/osc
	SH1106_Command(0x80);

	SH1106_Command(0xA8); // multiplex ratio
	SH1106_Command(0x3F); // 64 rows

	SH1106_Command(0xD3); // display offset
	SH1106_Command(0x00);

	SH1106_Command(0x40); // start line = 0

	SH1106_Command(0xAD); // DC-DC control
	SH1106_Command(0x8B); // enable internal DC-DC

	SH1106_Command(0xA1); // segment remap
	SH1106_Command(0xC8); // COM scan direction remap

	SH1106_Command(0xDA); // COM pins hardware config
	SH1106_Command(0x12);

	SH1106_Command(0x81); // contrast
	SH1106_Command(0x80);

	SH1106_Command(0xD9); // pre-charge
	SH1106_Command(0x22);

	SH1106_Command(0xDB); // VCOM deselect level
	SH1106_Command(0x35);

	SH1106_Command(0xA4); // output follows RAM
	SH1106_Command(0xA6); // normal display

	SH1106_Command(0xAF); // display on
}

void SH1106_Clear(void)
{
	for (uint8_t page = 0; page < 8; page++) {
		SH1106_SetPageColumn(page, 2); // many SH1106 modules use a 2-column offset
		for (uint8_t col = 0; col < 128; col++) {
			SH1106_Data(0x00);
		}
	}
}

void SH1106_Fill(void)
{
	for (uint8_t page = 0; page < 8; page++) {
		SH1106_SetPageColumn(page, 2); // often needed on SH1106
		for (uint8_t col = 0; col < 128; col++) {
			SH1106_Data(0xFF);
		}
	}
}
