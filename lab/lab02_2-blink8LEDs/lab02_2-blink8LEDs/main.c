/*
 * lab02_2-blink8LEDs.c
 *
 * Created: 2/9/2026 10:36:05 AM
 * Author : samou
 */ 

#define F_CPU 4000000UL

#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>

#define DIRSET_gm 0xFF
#define OUTTGL_gm 0xFF
#define OUTSET_gm 0xFF
#define OUTCLR_gm 0xFF
#define BLINK_DURATION 2000

void blink(uint8_t freq, uint8_t PINx_bm);
void delay_ms_var(uint16_t ms);


int main(void)
{
	VPORTD.DIRSET = DIRSET_gm;
	VPORTD.OUTCLR = OUTCLR_gm;
	
	uint8_t PINx_bm[] = {PIN0_bm, PIN1_bm, PIN2_bm, PIN3_bm, PIN4_bm, PIN5_bm, PIN6_bm, PIN7_bm};
	/* Replace with your application code */
	while (1)
	{
    for(uint8_t idx=0; ; idx = (idx+1) % 8){
	    blink(2, PINx_bm[idx]);
	    blink(4, PINx_bm[idx]);
    }
    return 0;               //should never hit this
	}
}

void delay_ms_var(uint16_t ms){
	while(ms--){
		_delay_ms(1);
	}
}

void blink(uint8_t freq, uint8_t PINx_bm){
	uint16_t half_period_ms = (1000 / 2) freq;     // recall T = 1/f
	for(int t=0; t<BLINK_DURATION; t+=half_period_ms){
		VPORTD.OUTTGL = PINx_bm;
		delay_ms_var(half_period_ms);
		VPORTD.OUTTGL = PINx_bm;
		delay_ms_var(half_period_ms);
	}
}


