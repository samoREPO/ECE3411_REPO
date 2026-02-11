/*
 * Lab06_1-HumanReacctionTime.c
 *
 * Created: 2/11/2026 2:42:33 PM
 * Author : samou
 */ 

#define F_CPU 16000000UL
#include "uart.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>

#define DIRSET_gm 0xFF
#define OUTCLR_gm 0xFF

volatile int timerCount;

void init_clock();
void InitTCA0 (void);

int main(void)
{
	PORTD.DIRSET = DIRSET_gm;
	
	InitTCA0();
	
	while (1) 
    {
		PORTD.OUTCLR = OUTCLR_gm;
		long r = (random() % 3000) + 2000; // random: (2000 - 5000)
		timerCount = r;
		if(timerCount == 0){
			PORTD.OUTSET = PIN7_bm;
		}
    }
}

ISR(TCA0_OVF_vect){
	if(timerCount>0) timerCount--;
	TCA0.SINGLE.INTFLAGS |= TCA_SINGLE_OVF_bm; // must clear the interrupt	
}

// 1ms ISR for Timer TCA0 assuming F_CPU = 16MHz
void InitTCA0 (void){
	TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc; // Normal mode
	TCA0.SINGLE.PER = 249;						// Set number of ticks for period
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;	// Enable TCA0 Overflow ISR
	
	// Set Prescalar to 64 & enable timer. Each tick is 4us
	TCA0.SINGLE.CTRLA |= (TCA_SINGLE_CLKSEL_DIV64_gc | TCA_SINGLE_ENABLE_bm);
}


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