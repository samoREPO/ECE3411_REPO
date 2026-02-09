/*
 * LabExam01.c
 *
 * Created: 2/9/2026 1:33:43 PM
 * Author : samou
 */ 

#define F_CPU 4000000UL // set CLK = 4MHz

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#define DIRSET_gm 0xFF
#define OUTTGL_gm 0xFF
#define OUTSET_gm 0xFF
#define OUTCLR_gm 0xFF

#define BUTTON_1 PIN2_bm
#define BUTTON_2 PIN4_bm

void blink(uint8_t freq, uint8_t PINx_bm);
void delay_ms_var(uint16_t ms);


int main(void)
{
	VPORTD.DIRSET = DIRSET_gm;
    VPORTD.OUTCLR = OUTCLR_gm;
	PORTB.PIN2CTRL |= PORT_PULLUPEN_bm;
	
    while (1) 
    {
		blink(2, PIN3_bm);
    }
}

void delay_ms_var(uint16_t ms){
	while(ms--){
		_delay_ms(1);
	}
}

void blink(uint8_t freq, uint8_t PINx_bm){
	uint16_t half_period_ms = (1000 / 2) freq;     // recall T = 1/f
	VPORTD.OUTTGL = PINx_bm;
	delay_ms_var(half_period_ms);
	VPORTD.OUTTGL = PINx_bm;
	delay_ms_var(half_period_ms);
}

