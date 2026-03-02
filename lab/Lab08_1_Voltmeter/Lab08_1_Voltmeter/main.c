/*
 * Lab08_1_Voltmeter.c
 *
 * Created: 3/2/2026 9:06:21 AM
 * Author : samou
 */ 

#define F_CPU 16000000UL // CPUclk = 16MHz

#include "uart.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

void init_ADC();
void init_clock();
void read_ADC_then_print();
void InitTCA0();

volatile bool ready_ADC = false;

int main(void)
{
	// UART setup
	init_clock();
	uart_init(3,9600,NULL);
	InitTCA0();
	sei();
	
	// ADC setup
	init_ADC();
	
    while (1) 
    {
		if(ready_ADC){
			cli();
			ready_ADC = false;
			sei();
			read_ADC_then_print();
		}
		
    }
}

//----------------------- UART Functions ------------------------//
void init_clock() {

	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.XOSCHFCTRLA =  CLKCTRL_FRQRANGE_16M_gc | CLKCTRL_ENABLE_bm;
	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.MCLKCTRLA = CLKCTRL_CLKSEL_EXTCLK_gc;

	while(!(CLKCTRL.MCLKSTATUS & CLKCTRL_EXTS_bm));

}

void InitTCA0(){
	TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc; // Normal mode
	TCA0.SINGLE.PER = 999; // Set number of ticks per period
	
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm; // Enable TCA0 Overflow ISR
	
	TCA0.SINGLE.CTRLA |= TCA_SINGLE_CLKSEL_DIV16_gc; // Set Prescalar to 16. each tick is 1us.
	TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm; // enable timer
}

ISR(TCA0_OVF_vect){
	ready_ADC = true;
	TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
}
//--------------------- ADC functions-----------------------//
void init_ADC(){
	ADC0.MUXPOS = ADC_MUXPOS_AIN8_gc; // Input voltage at AIN8 PORTE0
	ADC0.CTRLC = ADC_PRESC_DIV16_gc; // Set CLK_ADC to 1MHz (valid range from 125kHz to 2MHz)
	ADC0.CTRLD = ADC_INITDLY_DLY16_gc; // Delay 16 CLK_ADC cycles - 16 us
	VREF.ADC0REF = VREF_REFSEL_VDD_gc; // VDD as reference
	ADC0.CTRLA = ADC_ENABLE_bm | ADC_LEFTADJ_bm; // enables the ADC circuitry and left adjusted and 12-bit (3.3/2^12=0.8mV resolution 
	ADC0.COMMAND = ADC_STCONV_bm; // start first A to D conversion
	 
	while(ADC0.COMMAND & ADC_STCONV_bm);
}

void read_ADC_then_print(){
	uint16_t r = ADC0.RES; // TODO: check if i can do this instead of RESL then RESH
	float Voltage = ((float)r/65536.0f) * 3.3f; // 3.3*(RES/(2^12))
	
	ADC0.COMMAND = ADC_STCONV_bm; // start next A to D conversion
	
	char VoltageBuffer[6];
	dtostrf(Voltage, 4, 2, VoltageBuffer);
	printf("%s\n\r", VoltageBuffer);
}