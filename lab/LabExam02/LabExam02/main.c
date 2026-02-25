/*
 * LabExam02.c
 *
 * Created: 2/25/2026 1:36:21 PM
 * Author : samou
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>

// blink control variables
uint8_t frequency = 2;
uint8_t position = 3;
uint8_t pin[] = {PIN0_bm, PIN1_bm, PIN2_bm, PIN3_bm, PIN4_bm, PIN5_bm, PIN6_bm, PIN7_bm};

void delay_ms_var(uint16_t ms);
void blink(uint8_t frequency, uint8_t position);

int main(void)
{
	// LED setup
	PORTD.DIRSET = 0xFF;
	PORTD.OUTCLR = 0xFF;
	
    /* Replace with your application code */
    while (1) 
    {
		blink(frequency, position);
    }
	
}

// Delay function that allows non compile-time constant arguments
void delay_ms_var(uint16_t ms){
	for (int i=0; i<ms; i++)
	_delay_ms(1);
}

// 50% duty cycle blink function
void blink(uint8_t frequency, uint8_t position){
	uint16_t half_period_ms = 1000 / (2*frequency);     // recall T = 1/f
	PORTD.OUTTGL = pin[position];
	delay_ms_var(half_period_ms);
	PORTD.OUTTGL = pin[position];
	delay_ms_var(half_period_ms);
}