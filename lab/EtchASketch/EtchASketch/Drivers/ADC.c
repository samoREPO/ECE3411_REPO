/*
 * ADC.c
 *
 * Created: 4/20/2026 2:45:35 PM
 *  Author: samou
 */ 


#include "ADC.h"

#include <avr/io.h>

void ADC_Init(void)
{
	ADC0.MUXPOS = ADC_MUXPOS_AIN4_gc;

	ADC0.CTRLC = ADC_PRESC_DIV16_gc;      // 16 MHz / 16 = 1 MHz ADC clock
	ADC0.CTRLD = ADC_INITDLY_DLY16_gc;    // startup delay

	VREF.ADC0REF = VREF_REFSEL_VDD_gc;    // VDD reference

	ADC0.CTRLA |= ADC_ENABLE_bm;

	// Dummy conversion after enabling ADC
	ADC0.COMMAND = ADC_STCONV_bm;
	while (ADC0.COMMAND & ADC_STCONV_bm);
}

uint16_t ADC_ReadChannel(uint8_t muxpos)
{
	ADC0.MUXPOS = muxpos;

	ADC0.COMMAND = ADC_STCONV_bm;
	while (ADC0.COMMAND & ADC_STCONV_bm);

	return ADC0.RES;
}