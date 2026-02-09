/*
 * lab01_2-blinkPORTD.c
 *
 * Created: 2/9/2026 10:32:00 AM
 * Author : samou
 */ 

#define F_CPU 4000000UL  // 4 MHz system clock
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

#define DIRSET_gm 0xFF
#define OUTSET_gm 0xFF
#define OUTCLR_gm 0xFF


int main(void)
{
    /* Replace with your application code */
	VPORTD.DIRSET = DIRSET_gm;
	VPORTD.OUTSET = OUTSET_gm;
    while (1) 
    {
		VPORTD.OUTSET = OUTSET_gm;
		_delay_ms(1000);
		VPORTD.OUTCLR = OUTCLR_gm;
		_delay_ms(1000);
    }
	return(0);
}

