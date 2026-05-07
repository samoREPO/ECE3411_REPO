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
#include <string.h>

#define SH1106_CONTROL_CMD   0x00
#define SH1106_CONTROL_DATA  0x40

uint8_t sh1106_buffer[SH1106_BUF_SIZE];

static void SH1106_WriteCommand(uint8_t cmd)
{
	I2C_Address(SH1106_I2C_ADDR, I2C_WRITE);
	I2C_WriteByte(SH1106_CONTROL_CMD);
	I2C_WriteByte(cmd);
	I2C_Stop();
}

static void SH1106_WriteDataBlock(const uint8_t *data, uint16_t length)
{
	I2C_Address(SH1106_I2C_ADDR, I2C_WRITE);
	I2C_WriteByte(SH1106_CONTROL_DATA);

	for (uint16_t i = 0; i < length; i++)
	{
		I2C_WriteByte(data[i]);
	}

	I2C_Stop();
}

void SH1106_ClearBuffer(void)
{
	memset(sh1106_buffer, 0x00, sizeof(sh1106_buffer));
}

void SH1106_FillBuffer(void)
{
	memset(sh1106_buffer, 0xFF, sizeof(sh1106_buffer));
}

void SH1106_SetPixel(uint8_t x, uint8_t y)
{
	if (x >= SH1106_WIDTH || y >= SH1106_HEIGHT) return;

	uint16_t index = x + ((uint16_t)(y / 8) * SH1106_WIDTH);
	sh1106_buffer[index] |= (1 << (y % 8));
}

void SH1106_ClearPixel(uint8_t x, uint8_t y)
{
	if (x >= SH1106_WIDTH || y >= SH1106_HEIGHT) return;

	uint16_t index = x + ((uint16_t)(y / 8) * SH1106_WIDTH);
	sh1106_buffer[index] &= ~(1 << (y % 8));
}

void SH1106_WritePixel(uint8_t x, uint8_t y, uint8_t color)
{
	if (color)
	{
		SH1106_SetPixel(x, y);
	}
	else
	{
		SH1106_ClearPixel(x, y);
	}
}

void SH1106_UpdateScreen(void)
{
	for (uint8_t page = 0; page < SH1106_PAGES; page++)
	{
		SH1106_WriteCommand(0xB0 | page);  // page address
		SH1106_WriteCommand(0x02);         // lower column address
		SH1106_WriteCommand(0x10);         // upper column address

		SH1106_WriteDataBlock(&sh1106_buffer[page * SH1106_WIDTH], SH1106_WIDTH);
	}
}

void SH1106_InvertDisplay(uint8_t enable)
{
	if (enable)
	{
		SH1106_WriteCommand(0xA7);
	}
	else
	{
		SH1106_WriteCommand(0xA6);
	}
}

void SH1106_Init(void)
{
	_delay_ms(100);

	SH1106_WriteCommand(0xAE); // display off

	SH1106_WriteCommand(0xD5); // display clock
	SH1106_WriteCommand(0x80);

	SH1106_WriteCommand(0xA8); // multiplex ratio
	SH1106_WriteCommand(0x3F);

	SH1106_WriteCommand(0xD3); // display offset
	SH1106_WriteCommand(0x00);

	SH1106_WriteCommand(0x40); // display start line

	SH1106_WriteCommand(0xAD); // DC-DC control
	SH1106_WriteCommand(0x8B);

	SH1106_WriteCommand(0xA1); // segment remap
	SH1106_WriteCommand(0xC8); // COM scan direction

	SH1106_WriteCommand(0xDA); // COM pins
	SH1106_WriteCommand(0x12);

	SH1106_WriteCommand(0x81); // contrast
	SH1106_WriteCommand(0x7F);

	SH1106_WriteCommand(0xD9); // pre-charge period
	SH1106_WriteCommand(0x22);

	SH1106_WriteCommand(0xDB); // VCOMH
	SH1106_WriteCommand(0x20);

	SH1106_WriteCommand(0xA4); // display follows RAM
	SH1106_WriteCommand(0xA6); // normal display
	SH1106_WriteCommand(0xAF); // display on

	SH1106_ClearBuffer();
	SH1106_UpdateScreen();
}

uint8_t SH1106_GetPixel(uint8_t x, uint8_t y)
{
	if (x >= SH1106_WIDTH || y >= SH1106_HEIGHT) return 0;

	uint16_t index = x + ((uint16_t)(y / 8) * SH1106_WIDTH);
	return (sh1106_buffer[index] >> (y % 8)) & 1;
}

uint8_t countAliveNeighbours(uint8_t x, uint8_t y)
{
	uint8_t count = 0;

	for (int8_t dx = -1; dx <= 1; dx++)
	{
		for (int8_t dy = -1; dy <= 1; dy++)
		{
			if (dx == 0 && dy == 0) continue;

			uint8_t nx = (x + dx + SH1106_WIDTH) % SH1106_WIDTH;
			uint8_t ny = (y + dy + SH1106_HEIGHT) % SH1106_HEIGHT;

			count += SH1106_GetPixel(nx, ny);
		}
	}

	return count;
}

void SH1106_UpdateLife(void)
{
	uint8_t new_buffer[SH1106_BUF_SIZE];
	memset(new_buffer, 0x00, sizeof(new_buffer));

	for (uint8_t x = 0; x < SH1106_WIDTH; x++)
	{
		for (uint8_t y = 0; y < SH1106_HEIGHT; y++)
		{
			uint8_t alive = SH1106_GetPixel(x, y);
			uint8_t neighbors = countAliveNeighbours(x, y);

			uint8_t next_alive =
			(alive && (neighbors == 2 || neighbors == 3)) ||
			(!alive && neighbors == 3);

			if (next_alive)
			{
				uint16_t index = x + ((uint16_t)(y / 8) * SH1106_WIDTH);
				new_buffer[index] |= (1 << (y % 8));
			}
		}
	}

	memcpy(sh1106_buffer, new_buffer, sizeof(sh1106_buffer));
}