/*
 * ADC.c
 *
 * Created: 4/15/2026 9:12:27 AM
 *  Author: samou
 */ 

#include "ADC.h"
#include <avr/io.h>
#include <stdint.h>

uint16_t adc_read_channel(uint8_t muxpos)
{
    ADC0.MUXPOS = muxpos;

    // First conversion after mux switch: discard
    ADC0.COMMAND = ADC_STCONV_bm;
    while (ADC0.COMMAND & ADC_STCONV_bm) {}
    (void)ADC0.RES;

    // Second conversion: use this one
    ADC0.COMMAND = ADC_STCONV_bm;
    while (ADC0.COMMAND & ADC_STCONV_bm) {}

    return ADC0.RES;
}

void init_ADC(void)
{
    ADC0.CTRLC = ADC_PRESC_DIV16_gc;
    ADC0.CTRLD = ADC_INITDLY_DLY16_gc;
    VREF.ADC0REF = VREF_REFSEL_VDD_gc;
    ADC0.CTRLA = ADC_ENABLE_bm;
}

uint16_t read_ADC1(void)   // POT1
{
    return adc_read_channel(ADC_MUXPOS_AIN8_gc);
}

uint16_t read_ADC2(void)   // DPOT output
{
    return adc_read_channel(ADC_MUXPOS_AIN7_gc);
}

uint16_t read_ADC3(void)   // POT2
{
    return adc_read_channel(ADC_MUXPOS_AIN3_gc);
}

uint16_t read_ADC4(void)   // DAC output
{
    return adc_read_channel(ADC_MUXPOS_AIN0_gc);
}