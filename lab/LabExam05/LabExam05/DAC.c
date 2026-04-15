/*
 * DAC.c
 *
 * Created: 4/15/2026 2:24:28 PM
 *  Author: samou
 */ 

// -- DAC

#include "DAC.h"
#include <stdint.h>
#include <avr/io.h>

void InitDAC(void)
{
	DAC0.CTRLA = DAC_ENABLE_bm | DAC_OUTEN_bm;
	// Set DAC reference to VDD
	VREF.DAC0REF = VREF_REFSEL_VDD_gc;

}

void DAC_ouput(uint16_t sample)
{
	DAC0.DATAL = sample & 0x03; // write low 2 bits
	DAC0.DATAH =(sample >> 2) & 0xFF;
}