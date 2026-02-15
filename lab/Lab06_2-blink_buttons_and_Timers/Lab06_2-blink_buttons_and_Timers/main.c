/*
 * Lab06_2-blink_buttons_and_Timers.c
 *
 * Created: 2/14/2026 3:11:08 PM
 * Author : samou
 */ 

#define F_CPU 16000000 // CPUclk = 16 MHz
#define DEBOUNCE_TIME 10
#define PRINT_FREQUENCY_TIME 5000
#define FREQUENCY_TO_PERIOD_SCALAR 500
#define BTN1 (!(PORTB.IN & PIN2_bm))
#define BTN2 (!(PORTB.IN & PIN3_bm))

#include "uart.h"
#include <avr/io.h>
#include <stdbool.h>
#include <avr/interrupt.h>



typedef enum {RELEASED, MAYBE_PUSHED, PUSHED, MAYBE_RELEASED
	} btn_state_t;

volatile int debounce_timerCount = DEBOUNCE_TIME;
volatile int print_timerCount = PRINT_FREQUENCY_TIME;
volatile int blink_timerCount = FREQUENCY_TO_PERIOD_SCALAR / frequency;
volatile bool ready_to_print = false;
volatile bool ready_to_blink = false;
volatile bool btn1Pushed = false;
volatile bool btn2Pushed = false;

uint8_t frequency = 3;



// half_period_ms = 1000 / 2 / freq

void init_clock();
void buttonSM();
void handle_btn1_push(); // TODO implement this function body
void handle_btn2_push(); // TODO implement this function body
void handle_ready_to_print();
void toggle_LED();

int main(void)
{
	
	// UART setup
	init_clock();
	uart_init(3, 9600, NULL);

	// Button setup
	PORTB.DIRCLR = PIN2_bm;
	PORTB.PIN2CTRL |= PORT_PULLUPEN_bm;
	
	// LED setup
    PORTD.DIRSET = PIN7_bm;
	PORTD.OUTCLR = PIN7_bm;
	
	// Timer Setup
	InitTCA0();
	
	// Interrupt setup
	sei();
	
    while (1) 
    {
		if(btn1Pushed){
			cli();
			btn1Pushed = false;
			sei();
			handle_btn1_push();
		}
		if(btn2Pushed){
			cli();
			btn2Pushed = false;
			sei();
			handle_btn2_push();	
		}
		if(ready_to_print){
			cli();
			ready_to_print = false;
			sei();
			handle_ready_to_print();
		}
		if(ready_to_blink){
			cli();
			ready_to_blink = false;
			sei();
			toggle_LED();
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

ISR(TCA0_OVF_vect){
	
	// do i need separate timers?
	// or can i call both buttonSMs from the same timer?
	// BTN debounce timer
	if (debounce_timerCount >0){
		debounce_timerCount--;
	}
	else{
		buttonSM1();
		buttonSM2();
		debounce_timerCount = DEBOUNCE_TIME;
	}
	
	// 5 second timer for printing frequency
	if (print_timerCount > 0){
		print_timerCount--;
	}
	else{
		ready_to_print = true;
		print_timerCount = PRINT_FREQUENCY_TIME;
		
	}
	if(blink_timerCount>0){
		ready_to_blink = true;
		blink_timerCount = FREQUENCY_TO_PERIOD_SCALAR / frequency;
	}
	
	// "T/2" second timer for blinking
	
	// must clear the interrupt
	TCA0.SINGLE.INTFLAGS |= TCA_SINGLE_OVF_bm; 
}

void buttonSM1(){
	static btn_state_t btn_state1 = RELEASED;
	switch(btn_state1){
		case RELEASED:
			if(BTN1) btn_state1 = MAYBE_PUSHED;
			break;
		case MAYBE_PUSHED:
			if(BTN1){
				btn_state1 = PUSHED;
				btn1Pushed = true;
			}
			else btn_state1 = RELEASED;
			break;
		case PUSHED:
			if(!BTN1) btn_state1 = MAYBE_RELEASED;
			break;
		case MAYBE_RELEASED:
			if(BTN1) btn_state1 = PUSHED;
			else btn_state1 = RELEASED;
			break;
	}
	
}

void buttonSM2(){
	static btn_state_t btn_state2 = RELEASED;
	switch(btn_state2){
		case RELEASED:
		if(BTN2) btn_state2 = MAYBE_PUSHED;
		break;
		case MAYBE_PUSHED:
		if(BTN2){
			btn_state2 = PUSHED;
			btn2Pushed = true;
		}
		else btn_state2 = RELEASED;
		break;
		case PUSHED:
		if(!BTN2) btn_state2 = MAYBE_RELEASED;
		break;
		case MAYBE_RELEASED:
		if(BTN2) btn_state2 = PUSHED;
		else btn_state2 = RELEASED;
		break;
	}
	
}

void handle_btn1_push(){
	frequency++;
}

void handle_btn2_push(){
	frequency--;
}

void handle_ready_to_print(){
	printf("frequency: %d\n", frequency);
}

void toggle_LED(){
	PORTD.OUTTGL = PIN7_bm;
}
