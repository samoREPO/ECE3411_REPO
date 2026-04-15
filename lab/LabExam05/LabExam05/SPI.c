/*
 * SPI.c
 *
 * Created: 4/15/2026 9:20:15 AM
 *  Author: samou
 */ 


#include <avr/io.h>

#include "SPI.h"

#define SPI_PORT PORTA
#define SPI_MOSI PIN4_bm
#define SPI_MISO PIN5_bm
#define SPI_SCK  PIN6_bm
#define SPI_SS   PIN7_bm

void SPI_Host_Init(void)
{
	/* Set MOSI, SCK, and SS as outputs; MISO as input */
	SPI_PORT.DIRSET = SPI_MOSI | SPI_SCK | SPI_SS;
	SPI_PORT.DIRCLR = SPI_MISO;

	/* Deselect slave by default */
	SPI_PORT.OUTSET = SPI_SS;

	/* Enable SPI in host mode, fclk/128 */
	SPI0.CTRLA = SPI_ENABLE_bm | SPI_MASTER_bm | SPI_PRESC_DIV64_gc;
}

uint8_t SPI_Transfer(uint8_t data)
{
	SPI0.DATA = data;
	while (!(SPI0.INTFLAGS & SPI_IF_bm));
	return SPI0.DATA;
}

void SPI_Select(void)
{
	SPI_PORT.OUTCLR = SPI_SS;
}

void SPI_Deselect(void)
{
	SPI_PORT.OUTSET = SPI_SS;
}

void SPI_Client_Init(void)
{
	/* Set MISO output, all others input */
	SPI_PORT.DIRSET = SPI_MISO;
	SPI_PORT.DIRCLR = SPI_MOSI | SPI_SCK | SPI_SS;

	/* Enable SPI in client mode */
	SPI0.CTRLA = SPI_ENABLE_bm;
}

uint8_t SPI_Client_Receive(void)
{
	while (!(SPI0.INTFLAGS & SPI_IF_bm));
	return SPI0.DATA;
}