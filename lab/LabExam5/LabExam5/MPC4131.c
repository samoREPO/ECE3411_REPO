/*
 * MPC4131.c
 *
 * Created: 4/15/2026 9:22:06 AM
 *  Author: samou
 * Pure device-driver interface for the MCP4131 digital potentiometer.
 * This file only knows how to frame and send device commands.
*/

#include <stdint.h>

#include "MPC4131.h"
#include "SPI.h"

void MPC4131_write(uint16_t data)
{
	uint16_t frame = (data & 0x03FF);
	uint8_t frameH = ((frame & 0xFF00) >> 8);
	uint8_t frameL = frame & 0x00FF;

	SPI_Select();
	SPI_Transfer(frameH);
	SPI_Transfer(frameL);
	SPI_Deselect();
}