/*
 * lab02_1-blinkLED.c
 *
 * Created: 2/9/2026 10:34:58 AM
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

void blink(uint8_t freq);
void delay_ms_var(uint16_t ms);


int main(void)
{
	VPORTD.DIRSET = DIRSET_gm;
	VPORTD.OUTCLR = OUTCLR_gm;
    /* Replace with your application code */
    while (1) 
    {
		blink(2);
		blink(4);
    }
}

void delay_ms_var(uint16_t ms){
	while(ms--){
		_delay_ms(1);
	}
}

void blink(uint8_t freq){
    uint16_t half_period_ms = (1000 / 2) freq;     // recall T = 1/f
	for(int t=0; t<BLINK_DURATION; t+=half_period_ms){
		VPORTD.OUTTGL = OUTTGL_gm;
		delay_ms_var(half_period_ms);
		VPORTD.OUTTGL = OUTTGL_gm;
		delay_ms_var(half_period_ms);
	}
}


