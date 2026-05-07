/*
 * button.c
 *
 * Created: 4/20/2026 2:48:49 PM
 *  Author: samou
 */ 

#include <avr/io.h>

void init_buttons(void)
{
	/* Buttons on PORTB, active low with pull-ups */
	PORTB.DIRCLR = BTN_CLEAR_bm | BTN_MODE_bm;
	PORTB.PIN2CTRL = PORT_PULLUPEN_bm;
	PORTB.PIN3CTRL = PORT_PULLUPEN_bm;
}