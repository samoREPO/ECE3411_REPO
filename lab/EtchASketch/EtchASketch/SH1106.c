/*
 * SH1106.c
 *
 * Created: 4/4/2026 11:24:59 AM
 *  Author: samou
 */ 

#define F_CPU 16000000UL

#include "SH1106.h"
#include "I2C.h"

#include <avr/io.h>
#include <util/delay.h>
#include <util/twi.h>
#include <string.h>

#define SH1106_CONTROL_CMD   0x00
#define SH1106_CONTROL_DATA  0x40

uint8_t sh1106_buffer[SH1106_BUF_SIZE];

static void SH1106_WriteCommand(uint8_t cmd)
{
	TWI_Address(SH1106_I2C_ADDR, TW_WRITE);
	TWI_Transmit_Data(SH1106_CONTROL_CMD);
	TWI_Transmit_Data(cmd);
	TWI_Stop();
}

static void SH1106_WriteDataBlock(uint8_t *data, uint16_t len)
{
	TWI_Address(SH1106_I2C_ADDR, TW_WRITE);
	TWI_Transmit_Data(SH1106_CONTROL_DATA);

	for (uint16_t i = 0; i < len; i++)
	{
		TWI_Transmit_Data(data[i]);
	}

	TWI_Stop();
}

void SH1106_ClearBuffer(void)
{
	memset(sh1106_buffer, 0, sizeof(sh1106_buffer));
}

void SH1106_SetPixel(uint8_t x, uint8_t y)
{
	if (x >= SH1106_WIDTH || y >= SH1106_HEIGHT) return;

	uint16_t index = x + (y / 8) * SH1106_WIDTH;
	sh1106_buffer[index] |= (1 << (y % 8));
}

void SH1106_ClearPixel(uint8_t x, uint8_t y)
{
	if (x >= SH1106_WIDTH || y >= SH1106_HEIGHT) return;

	uint16_t index = x + (y / 8) * SH1106_WIDTH;
	sh1106_buffer[index] &= ~(1 << (y % 8));
}

void SH1106_UpdateScreen(void)
{
	for (uint8_t page = 0; page < SH1106_PAGES; page++)
	{
		SH1106_WriteCommand(0xB0 | page);   // page address
		SH1106_WriteCommand(0x02);          // lower column start
		SH1106_WriteCommand(0x10);          // upper column start

		SH1106_WriteDataBlock(&sh1106_buffer[page * SH1106_WIDTH], SH1106_WIDTH);
	}
}

void SH1106_Init(void)
{
	_delay_ms(100);

	SH1106_WriteCommand(0xAE); // display off
	SH1106_WriteCommand(0xD5); // set display clock divide ratio/osc freq
	SH1106_WriteCommand(0x80);

	SH1106_WriteCommand(0xA8); // multiplex ratio
	SH1106_WriteCommand(0x3F); // 1/64

	SH1106_WriteCommand(0xD3); // display offset
	SH1106_WriteCommand(0x00);

	SH1106_WriteCommand(0x40); // start line = 0

	SH1106_WriteCommand(0xAD); // DC-DC control
	SH1106_WriteCommand(0x8B); // DC-DC on

	SH1106_WriteCommand(0xA1); // segment remap
	SH1106_WriteCommand(0xC8); // COM scan direction remapped

	SH1106_WriteCommand(0xDA); // COM pins config
	SH1106_WriteCommand(0x12);

	SH1106_WriteCommand(0x81); // contrast
	SH1106_WriteCommand(0x7F);

	SH1106_WriteCommand(0xD9); // pre-charge
	SH1106_WriteCommand(0x22);

	SH1106_WriteCommand(0xDB); // vcomh
	SH1106_WriteCommand(0x20);

	SH1106_WriteCommand(0xA4); // output follows RAM
	SH1106_WriteCommand(0xA6); // normal display
	SH1106_WriteCommand(0xAF); // display on

	SH1106_ClearBuffer();
	SH1106_UpdateScreen();
}