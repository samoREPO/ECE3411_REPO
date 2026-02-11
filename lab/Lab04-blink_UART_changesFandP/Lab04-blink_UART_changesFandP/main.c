/*
 * Lab04-blink_UART_changesFandP.c
 *
 * Created: 2/9/2026 10:59:23 AM
 * Author : samou
 */ 

#define F_CPU 16000000UL
#include "uart.h"
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#define REG_ON 0b11111111   // mask for setting all bits of a register HIGH
#define REG_OFF 0b00000000  // mask for setting all bits of a register LOW

void init_clock() {

	CPU_CCP = CCP_IOREG_gc;

	CLKCTRL.XOSCHFCTRLA =  CLKCTRL_FRQRANGE_16M_gc | CLKCTRL_ENABLE_bm;

	CPU_CCP = CCP_IOREG_gc;

	CLKCTRL.MCLKCTRLA = CLKCTRL_CLKSEL_EXTCLK_gc;

	while(!(CLKCTRL.MCLKSTATUS & CLKCTRL_EXTS_bm));

}

// Delay function that allows non compile-time constant arguments
void delay_ms_var(uint16_t ms){
    for (int i=0; i<ms; i++)
    _delay_ms(1);
}

// 50% duty cycle blink function
void blink(uint8_t curr_freq, uint8_t pin){
	uint16_t half_period_ms = 1000 / (2*curr_freq);     // recall T = 1/f
	PORTD.OUTTGL = pin;
	delay_ms_var(half_period_ms);
	PORTD.OUTTGL = pin;
	delay_ms_var(half_period_ms);
}


int main(void)
{
	PORTD.DIRSET = REG_ON;     // Initialize register D's mode to OUT
	PORTD.OUTCLR = REG_ON;   // Initialize register D's value to 0
	
	uint8_t pin[] = {PIN0_bm, PIN1_bm, PIN2_bm, PIN3_bm, PIN4_bm, PIN5_bm, PIN6_bm, PIN7_bm};
	uint8_t curr_pin = PIN2_bp;

	uint8_t curr_freq = 2; // curr_freq: {(1, 'inf')}
	uint16_t counter = 0; // count up to 5000 ms
	
	init_clock();
	
	uart_init(3, 9600, NULL);
	
	/* Replace with your application code */
	while (1)
	{
		// ---- blink ---- //
		blink(curr_freq, pin[curr_pin]);			// oscillate pin at {curr_freq} Hz for 2s
		counter++;
		
		if(counter == 5*curr_freq){
			char FP_sel;
			printf("Do you want to change the frequency or position? (F/P): ");
			scanf("%c", &FP_sel);
			
			if(FP_sel == 'F' || FP_sel == 'f'){
				printf("Frequency: ");
				int tmpF;
				scanf("%d", &tmpF);
				if(tmpF >= 1 || tmpF < 10) curr_freq = tmpF;
			}
			else if(FP_sel == 'P' || FP_sel == 'p'){
				printf("Position: ");
				int tmpP;
				scanf("%d", &tmpP);
				if(tmpP>=0 || tmpP<8) curr_pin = tmpP;
			}
			else{
				printf("INVALID CHARACTER\nTRY AGAIN\n");
			}
			counter = 0;
		}
	}
}
