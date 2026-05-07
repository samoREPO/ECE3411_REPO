/*
 * I2C.c
 *
 * Created: 4/4/2026 11:54:36 AM
 *  Author: samou
 */ 

#include "I2C.h"

#include <avr/io.h>

void I2C_Init(void)
{
	// Approx 80 kHz at 16 MHz system clock
	TWI0.MBAUD = 95;

	// Internal pullups. External pullups are still preferred for real hardware.
	PORTA.PIN2CTRL |= PORT_PULLUPEN_bm;
	PORTA.PIN3CTRL |= PORT_PULLUPEN_bm;

	TWI0.MCTRLA = TWI_ENABLE_bm;
	TWI0.MSTATUS = TWI_BUSSTATE_IDLE_gc;
}

void I2C_Stop(void)
{
	TWI0.MCTRLB = TWI_MCMD_STOP_gc;
}

uint8_t I2C_Address(uint8_t address, uint8_t mode)
{
	TWI0.MADDR = (address << 1) | mode;

	if (mode == I2C_WRITE)
	{
		while (!(TWI0.MSTATUS & TWI_WIF_bm));
	}
	else
	{
		while (!(TWI0.MSTATUS & TWI_RIF_bm));
	}

	if (TWI0.MSTATUS & TWI_RXACK_bm)
	{
		I2C_Stop();
		return 1;
	}

	if (TWI0.MSTATUS & (TWI_ARBLOST_bm | TWI_BUSERR_bm))
	{
		I2C_Stop();
		return 1;
	}

	return 0;
}

uint8_t I2C_WriteByte(uint8_t data)
{
	TWI0.MDATA = data;

	while (!(TWI0.MSTATUS & TWI_WIF_bm));

	if (TWI0.MSTATUS & (TWI_ARBLOST_bm | TWI_BUSERR_bm))
	{
		I2C_Stop();
		return 1;
	}

	if (TWI0.MSTATUS & TWI_RXACK_bm)
	{
		I2C_Stop();
		return 1;
	}

	return 0;
}

uint8_t I2C_ReadByteNACK(void)
{
	while (!(TWI0.MSTATUS & TWI_RIF_bm));

	uint8_t data = TWI0.MDATA;

	TWI0.MCTRLB = TWI_ACKACT_NACK_gc | TWI_MCMD_STOP_gc;

	return data;
}

uint8_t I2C_Write(uint8_t address, uint8_t data)
{
	if (I2C_Address(address, I2C_WRITE)) return 1;
	if (I2C_WriteByte(data)) return 1;

	I2C_Stop();
	return 0;
}

uint8_t I2C_WriteBytes(uint8_t address, const uint8_t *data, uint16_t length)
{
	if (I2C_Address(address, I2C_WRITE)) return 1;

	for (uint16_t i = 0; i < length; i++)
	{
		if (I2C_WriteByte(data[i])) return 1;
	}

	I2C_Stop();
	return 0;
}