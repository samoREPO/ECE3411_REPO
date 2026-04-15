/*
 * ADC.c
 *
 * Created: 4/15/2026 9:12:27 AM
 *  Author: samou
 */ 

#define F_CPU 16000000UL

#include "ADC.h"
#include <avr/io.h>
#include <stdint.h>

#define ADC_MAX 4095.0


void init_ADC(void)
{
	ADC0.MUXPOS = ADC_MUXPOS_AIN8_gc; // Input voltage at AIN8 PORTE0
	ADC0.CTRLC = ADC_PRESC_DIV16_gc; // Set CLK_ADC to 1MHz (valid range from 125kHz to 2MHz)
	ADC0.CTRLD = ADC_INITDLY_DLY16_gc; // Delay 16 CLK_ADC cycles - 16 us
	VREF.ADC0REF = VREF_REFSEL_VDD_gc; // VDD as reference
	ADC0.CTRLA |= ADC_ENABLE_bm; // enables the ADC circuitry and left adjusted and 12-bit (3.3/2^12=0.8mV resolution
	ADC0.COMMAND = ADC_STCONV_bm; // start first A to D conversion
	
	while(ADC0.COMMAND & ADC_STCONV_bm);
}

uint16_t read_ADC1(void)
{
	ADC0.MUXPOS = ADC_MUXPOS_AIN8_gc;
	ADC0.COMMAND = ADC_STCONV_bm; // start next A to D conversion
	loop_until_bit_is_clear(ADC0.COMMAND, ADC_STCONV_bp);
		
	return ADC0.RES;
}

uint16_t read_ADC2(void)
{
	ADC0.MUXPOS = ADC_MUXPOS_AIN7_gc;
	ADC0.COMMAND = ADC_STCONV_bm; // start next A to D conversion
	loop_until_bit_is_clear(ADC0.COMMAND, ADC_STCONV_bp);
	
	return ADC0.RES;
}