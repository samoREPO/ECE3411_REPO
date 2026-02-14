/*
 * Lab06_1-HumanReacctionTime.c
 *
 * Created: 2/11/2026 2:42:33 PM
 * Author : samou
 */ 

#define F_CPU 16000000UL // CPUclk = 16MHZ
#define BTN (!(PORTB.IN & PIN2_bm))
#define DEBOUNCE_TIME 10
#include "uart.h"
#include <avr/interrupt.h> // includes interrupt
#include <stdlib.h> // includes support for PRNG
#include <stdbool.h> // includes typedef of bool & related functions

typedef enum {RELEASED, MAYBE_PUSHED, PUSHED, MAYBE_RELEASED
} btn_state_t;

typedef enum {IDLE, WAITING_TIMER, WAITING_REACTION, RESULTS
} ui_state_t;

void init_clock();
void InitTCA0();
void buttonSM();
void handle_btn_push();
void handle_waiting_timer();

volatile long msTimerCountdownRandom;
volatile long msTimerCountupReaction;

volatile uint8_t debounce_timerCount = DEBOUNCE_TIME;
volatile bool btnPushed = false;
volatile ui_state_t ui_state = IDLE;
volatile int result = 0;

int main(void)
{
	// UART setup
	init_clock();
	uart_init(3, 9600, NULL);
	//USART3.CTRLA |= USART_RXCIE_bm;
	
	// Button setup
	PORTB.DIRCLR = PIN2_bm;
	PORTB.PIN2CTRL |= PORT_PULLUPEN_bm;
	
	// LED setup
	PORTD.DIRSET = PIN7_bm;
	
	// Timer Setup
	InitTCA0();
	
	printf("Reaction Timer\n");
	printf("Press button to start\n");
	sei(); // Enable interrupts
	
	while(1){
			bool pressed;
			cli();
			pressed = btnPushed;
			btnPushed = false;
			sei();

			if (pressed) {
				handle_btn_push();   // interrupts ON while printing
			}

			if (ui_state == WAITING_TIMER && msTimerCountdownRandom == 0) {
				handle_waiting_timer();
			}
	}
}

//---------------------------//
//----- UART FUNCTIONS -----//
//-------------------------//
void init_clock() {

	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.XOSCHFCTRLA =  CLKCTRL_FRQRANGE_16M_gc | CLKCTRL_ENABLE_bm;
	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.MCLKCTRLA = CLKCTRL_CLKSEL_EXTCLK_gc;

	while(!(CLKCTRL.MCLKSTATUS & CLKCTRL_EXTS_bm));

}

//ISR(USART3_RXC_vect){
	//uint8_t rx_byte = USART3.RXDATAL;
//}
//----------------------------//

//----------------------------//
//----- TIMER FUNCTIONS -----//
//--------------------------//

// 1ms ISR for Timer TCA0 assuming F_CPU = 16MHz
void InitTCA0(){
	TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc; // Normal mode
	TCA0.SINGLE.PER = 249; // Set number of ticks per period
	
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm; // Enable TCA0 Overflow ISR
	
	TCA0.SINGLE.CTRLA |= TCA_SINGLE_CLKSEL_DIV64_gc; // Set Prescalar to 64. each tick is 4us.
	TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm; // enable timer
}

ISR(TCA0_OVF_vect){
	if (debounce_timerCount > 0){
		debounce_timerCount--;
	}
	else{
		buttonSM();
		debounce_timerCount = DEBOUNCE_TIME;
	}
	if (msTimerCountdownRandom > 0) {msTimerCountdownRandom--;}
	if (ui_state == WAITING_REACTION) { msTimerCountupReaction++; }
	TCA0.SINGLE.INTFLAGS |= TCA_SINGLE_OVF_bm; // Must clear the interrupt
}

void handle_waiting_timer(){
	switch(ui_state){
		case IDLE:
			break;
		case WAITING_TIMER:
			msTimerCountupReaction = 0;
			PORTD.OUTSET = PIN7_bm;
			ui_state = WAITING_REACTION;
			break;
		case WAITING_REACTION:
			break;
		case RESULTS:
			break;
	}
}
//---------------------------//

//---------------------------//
//----- BUTTON FUCTIONS -----//
//---------------------------//

void buttonSM(){
	static btn_state_t btn_state = RELEASED;
	switch (btn_state){
		case RELEASED:
			if (BTN){btn_state = MAYBE_PUSHED;}
			break;
		case MAYBE_PUSHED:
			if (BTN){
				btn_state = PUSHED;
				btnPushed = true;
			}
			else {
				btn_state = RELEASED;
			}
			break;
		case PUSHED:
			if (!BTN){btn_state = MAYBE_RELEASED;}
			break;
		case MAYBE_RELEASED:
			if (BTN){btn_state = PUSHED;}
			else {btn_state = RELEASED;}
			break;
	}
}

void handle_btn_push(){
	// TODO: Define button handler
		// what state changes does a button make?
		
	switch(ui_state){
		case IDLE:
			printf("Go!\n");
			printf("(Press button to stop the timer)\n");
			msTimerCountdownRandom = (random() % 3000) + 2000;
			ui_state = WAITING_TIMER;
		break;
		case WAITING_TIMER:
		break;
		case WAITING_REACTION:
			result = msTimerCountupReaction;
			PORTD.OUTCLR = PIN7_bm;
			printf("Reaction Time %d(ms)\n", result);
			if(result < 300) printf("Wow!\n");
			printf("Play again?\n");
			printf("(Press button to continue)\n");
			ui_state = RESULTS;
		break;
		case RESULTS:
			msTimerCountupReaction = 0;
			printf("Get Ready\n");
			printf("(push button to start)\n");
			ui_state = IDLE;
		break;
	}
}