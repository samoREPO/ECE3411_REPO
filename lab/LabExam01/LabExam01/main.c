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
	PORTD.DIRSET = DIRSET_gm; // task a
    PORTD.OUTCLR = OUTCLR_gm;
	PORTB.PIN2CTRL |= PORT_PULLUPEN_bm;
	
	uint16_t curr_freq = 2;
	uint8_t curr_byte_value = 0x04;
	
    while (1) 
    {
		//blink(2, PIN3_bm); // task b
		
		// is overflow handling automatic?
		PORTD.OUTSET = curr_byte_value++;
		delay_ms_var(1000 / curr_freq);
		
		
    }
}

void delay_ms_var(uint16_t ms){
	while(ms--){
		_delay_ms(1);
	}
}

void blink(uint8_t freq, uint8_t PINx_bm){
	uint16_t half_period_ms = (1000 / 2) / freq;     // recall T = 1/f
	PORTD.OUTTGL = PINx_bm;
	delay_ms_var(half_period_ms);
	PORTD.OUTTGL = PINx_bm;
	delay_ms_var(half_period_ms);
}

