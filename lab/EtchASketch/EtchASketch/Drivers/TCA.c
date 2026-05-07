/*
 * TCA.c
 *
 * Created: 4/25/2026 9:32:08 AM
 *  Author: samou
 */ 

#include "TCA.h"

#include <avr/io.h>
#include <avr/interrupt.h>

static volatile uint32_t tca0_millis = 0;

void TCA0_Init_1ms(void)
{
	TCA0.SINGLE.CTRLA = 0;
	TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc;

	// 16 MHz / 16 = 1 MHz timer clock
	// 1000 ticks = 1 ms
	TCA0.SINGLE.PER = 999;

	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;
	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV16_gc | TCA_SINGLE_ENABLE_bm;
}

uint32_t TCA0_Millis(void)
{
	uint32_t copy;

	cli();
	copy = tca0_millis;
	sei();

	return copy;
}

uint8_t TCA0_HasElapsed(uint32_t *previous_time, uint32_t interval_ms)
{
	uint32_t now = TCA0_Millis();

	if ((now - *previous_time) >= interval_ms)
	{
		*previous_time = now;
		return 1;
	}

	return 0;
}

ISR(TCA0_OVF_vect)
{
	tca0_millis++;
	TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
}