/*
 * Lab07_1-blink_Waveform.c
 *
 * Created: 2/16/2026 3:11:04 PM
 * Author : samou
 */ 

#define F_CPU 16000000UL // CPUclk = 16MHz
#define DEBOUNCE_TIME 10
#define PRINT_FREQUENCY_TIME 5000
#define FREQUENCY_TO_CMP0_SCALAR F_CPU/512
#define BTN1 (!(VPORTB.IN & PIN2_bm))
#define BTN2 (!(VPORTB.IN & PIN4_bm))

#include "uart.h"
#include <avr/io.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include <stdio.h>
	
volatile int debounce_timerCount1 = DEBOUNCE_TIME;
volatile int debounce_timerCount2 = DEBOUNCE_TIME;
volatile int print_frequency_timerCount = PRINT_FREQUENCY_TIME;
volatile bool btn1pushed = false;
volatile bool btn2pushed = false;
volatile bool ready_to_print = false;

uint8_t frequency = 1;

void init_clock();
void InitTCA0();
void InitTCA1();
void InitButtonInt();
void handle_btn1_push();
void handle_btn2_push();
void handle_ready_to_print();
void set_CMP0(uint8_t freq);

int main(void)
{

	init_clock();
	uart_init(3, 9600, NULL);

	InitTCA0();
	InitTCA1();
	InitButtonInt();
	
	set_CMP0(frequency);
	
	sei();
	
    while (1) 
    {
		if(btn1pushed){
			cli();
			btn1pushed = false;	
			sei();
			handle_btn1_push();
		}
		if(btn2pushed){
			cli();
			btn2pushed = false;
			sei();
			handle_btn2_push();
		}
		if(ready_to_print){
			cli();
			ready_to_print = false;
			sei();
			handle_ready_to_print();
		}
    }
}


void init_clock() {

	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.XOSCHFCTRLA =  CLKCTRL_FRQRANGE_16M_gc | CLKCTRL_ENABLE_bm;
	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.MCLKCTRLA = CLKCTRL_CLKSEL_EXTCLK_gc;

	while(!(CLKCTRL.MCLKSTATUS & CLKCTRL_EXTS_bm));

}

void InitTCA0(){
	TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc; // Normal mode
	TCA0.SINGLE.PER = 249; // Set number of ticks per period
	
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm; // Enable TCA0 Overflow ISR
	
	TCA0.SINGLE.CTRLA |= TCA_SINGLE_CLKSEL_DIV64_gc; // Set Prescalar to 64. each tick is 4us.
	TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm; // enable timer
}

void InitTCA1(){
	TCA1.SINGLE.CTRLB = TCA_SINGLE_WGMODE_FRQ_gc; // FREQ mode
	TCA1.SINGLE.CMP0 = (FREQUENCY_TO_CMP0_SCALAR / frequency) - 1; // Set number of ticks per period
	//TCA1.SINGLE.INTCTRL = TCA_SINGLE_CMP0_bm; // Enable TCA1 CMP0 ISR
	TCA1.SINGLE.CTRLB |= TCA_SINGLE_CMP0EN_bm; // Enable Waveform output
	
	// set prescalar to 64 & enable timer. Each tick is 4us
	TCA1.SINGLE.CTRLA |= (TCA_SINGLE_CLKSEL_DIV256_gc | TCA_SINGLE_ENABLE_bm);
	
	PORTMUX.TCAROUTEA |= PORTMUX_TCA1_PORTC_gc; // Route TCA1 to PORTC
	PORTC.DIRSET = PIN4_bm; // Enable waveform output
}

//-------------------- BUTTON things --------------------------------------//
void InitButtonInt(){
	PORTB.DIRCLR = (PIN2_bm | PIN4_bm); // enable BUTTON1 and BUTTON2 as inputs
	// setup BUTTON1
	PORTB.PIN2CTRL |= PORT_ISC_FALLING_gc;
	PORTB.PIN2CTRL |= PORT_PULLUPEN_bm;
	// setup BUTTON2 
	PORTB.PIN4CTRL |= PORT_ISC_FALLING_gc;	
}

ISR(PORTB_PORT_vect){
	if(PORTB.INTFLAGS & PIN2_bm){
		PORTB.INTFLAGS = PIN2_bm;  // TODO: check if this is the correct way to reset the flag
		if(debounce_timerCount1 == 0){debounce_timerCount1 = DEBOUNCE_TIME;}
	}
	if(PORTB.INTFLAGS & PIN4_bm){
		PORTB.INTFLAGS = PIN4_bm;  // TODO: check if this is the correct way to reset the flag
		if(debounce_timerCount2 == 0){debounce_timerCount2 = DEBOUNCE_TIME;}
	}
}

ISR(TCA0_OVF_vect){
	if(debounce_timerCount1 > 0){
		debounce_timerCount1--;
		if((debounce_timerCount1 == 0) && BTN1){btn1pushed = true;}
	}
	if(debounce_timerCount2 > 0){
		debounce_timerCount2--;
		if((debounce_timerCount2 == 0) && BTN2) {btn2pushed = true;}
	}
	
	// 5 second timer for printing frequency
	if (print_frequency_timerCount > 0){print_frequency_timerCount--;}
	else{
		ready_to_print = true;
		print_frequency_timerCount = PRINT_FREQUENCY_TIME;
	}
	
	TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm; // reset interrupt flag
}

void handle_btn1_push(){
	frequency++;
	set_CMP0(frequency);
}

void handle_btn2_push(){
	if(frequency > 1) frequency--;
	set_CMP0(frequency);
}

void handle_ready_to_print(){
	printf("frequency: %d\n", frequency);
}

void set_CMP0(uint8_t freq){
	TCA1.SINGLE.CMP0 = (FREQUENCY_TO_CMP0_SCALAR / freq) - 1;
}