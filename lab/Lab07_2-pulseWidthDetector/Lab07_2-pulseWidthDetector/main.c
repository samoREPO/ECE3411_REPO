/*
 * Lab07_2-pulseWidthDetector.c
 *
 * Created: 2/26/2026 10:12:43 AM
 * Author : samou
 */ 

#define F_CPU 16000000UL // CPUclk = 16MHz
#define DEBOUNCE_TIME 10
#define PRINT_FREQUENCY_TIME 5000
#define PRINT_PULSE_WIDTH_TIME 1000
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
volatile int print_pulse_width_timerCount = PRINT_PULSE_WIDTH_TIME;
volatile bool btn1pushed = false;
volatile bool btn2pushed = false;
volatile bool ready_to_print = false;
volatile bool ready_to_print_pulseWidth = false;
volatile uint8_t pulseWidth_Count_Reference = 0;
volatile uint32_t pulseWidth_Count = 0;
volatile uint32_t average_pulseWidth_Count = 0;

uint8_t frequency = 1;

void init_clock();
void InitTCA0();
void InitTCA1();
void InitButtonInt();
void InitReadWO0_Int();
void handle_btn1_push();
void handle_btn2_push();
void handle_ready_to_print();
void handle_print_pulseWidth();
void set_CMP0(uint8_t freq);


int main(void)
{
	// UART setup
	init_clock();
	uart_init(3, 9600, NULL);

	// Timer setup
	InitTCA0();
	sei();
	
	// WO0 setup
	InitTCA1();
	set_CMP0(frequency);
	InitReadWO0_Int();
	
	// Button setup
	InitButtonInt();
	
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
		if(ready_to_print_pulseWidth){
			cli();
			ready_to_print_pulseWidth = false;
			sei();
			handle_print_pulseWidth();
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

// --------------------- Timer Functions ------------------------//
void InitTCA0(){
	TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc; // Normal mode
	TCA0.SINGLE.PER = 999; // Set number of ticks per period
	
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm; // Enable TCA0 Overflow ISR
	
	TCA0.SINGLE.CTRLA |= TCA_SINGLE_CLKSEL_DIV16_gc; // Set Prescalar to 16. each tick is 1us.
	TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm; // enable timer
}

void InitTCA1(){
	TCA1.SINGLE.CTRLB = TCA_SINGLE_WGMODE_FRQ_gc; // FREQ mode
	TCA1.SINGLE.CMP0 = (FREQUENCY_TO_CMP0_SCALAR / frequency) - 1; // Set number of ticks per period
	//TCA1.SINGLE.INTCTRL = TCA_SINGLE_CMP0_bm; // Enable TCA1 CMP0 ISR
	TCA1.SINGLE.CTRLB |= TCA_SINGLE_CMP0EN_bm; // Enable Waveform output
	
	// set prescalar to 256 & enable timer. Each tick is 16us
	TCA1.SINGLE.CTRLA |= (TCA_SINGLE_CLKSEL_DIV256_gc | TCA_SINGLE_ENABLE_bm);
	
	PORTMUX.TCAROUTEA |= PORTMUX_TCA1_PORTC_gc; // Route TCA1 to PORTE
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
	if(PORTB.INTFLAGS & PIN5_bm){
		if(!(PORTB.IN & PIN5_bm)){
			// TODO: Capture time on rising edge
			pulseWidth_Count_Reference = TCA0.SINGLE.CNT;
			pulseWidth_Count = 0;
		}
		else{
			// TODO: Capture time on falling edge and take the difference
			pulseWidth_Count = pulseWidth_Count + TCA0.SINGLE.CNT - pulseWidth_Count_Reference;
			average_pulseWidth_Count += pulseWidth_Count;
		}
	}
}

ISR(TCA0_OVF_vect){
	pulseWidth_Count += 1000; // increment time counter by 1000us
	
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
	if (print_pulse_width_timerCount > 0) {print_pulse_width_timerCount--;}
	else{
		ready_to_print_pulseWidth = true;
		print_pulse_width_timerCount = PRINT_PULSE_WIDTH_TIME;
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

//------------------------------ Reading Pulse Width Things --------------------------------//
void InitReadWO0_Int(){
	PORTB.DIRCLR = (PIN5_bm); // enable pin5 as inputs
	PORTB.PIN5CTRL |= PORT_ISC_BOTHEDGES_gc;
	PORTB.PIN5CTRL |= PORT_PULLUPEN_bm;
}
void handle_print_pulseWidth(){
	printf("pulse width: %lu\n", (average_pulseWidth_Count / frequency));
}
