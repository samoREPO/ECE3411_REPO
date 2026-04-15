/*
 * AnalogComparator.c
 *
 * Created: 4/15/2026 9:25:58 AM
 *  Author: samou
 */ 

#include <avr/io.h>
#include <stdbool.h>

#include "AnalogComparator.h"

#define CMPSTATE_bm 0x10

void init_AC(void)
{
	AC0.CTRLA = AC_ENABLE_bm;
	/* compare pins PE0 (0,AINP1) and PD7 (0,AINN2) */
	AC0.MUXCTRL = (1 << AC_MUXPOS_gp) | (2 << AC_MUXNEG_gp);
}

bool AC_is_output_high(void)
{
	return (AC0.STATUS & CMPSTATE_bm) != 0;
}