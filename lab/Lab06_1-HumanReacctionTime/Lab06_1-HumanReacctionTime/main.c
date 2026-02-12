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
#include <stdbool.h>

#define DIRSET_gm 0xFF
#define OUTCLR_gm 0xFF

#define BTN (!(PORTD.IN & PIN2_bm)

typedef enum {RELEASED, MAYBE_PUSHED, PUSHED, MAYBE_RELEASED} btn_state_t;

typedef enum {IDLE, WAITING_TIMER, WAITING_INPUT, RESET} ui_state_t;
ui_state_t ui_state = IDLE;

volatile int timerCountdown, timerCountup;
volatile uint8_t rx_ready = 0; // Data received flag
volatile bool btnPushed = false;


void init_clock();
void InitTCA0 (void);
void handle_rx_ready();
void handle_timer_done();
void handle_btn_pressed();
void buttonSM();
void startup();

int main(void)
{
	PORTD.DIRSET = DIRSET_gm;
	
	InitTCA0();
	
	init_clock();
	uart_init(3, 9600, NULL);
	USART3.CTRLA |= USART_RXCIE_bm; // enable UART RX interrupt
	
	startup();
	
	sei(); // Enable interrupts
	
	while (1) 
    {
		if(btnPushed){
			btnPushed = 0;
			buttonSM();
		}
		if(rx_ready){
			cli();
			if(rx_ready) rx_ready = 0;
			handle_rx_ready();
			sei();
		}
		cli();
		handle_timer_done();
		sei();
		
    }
}

ISR(TCA0_OVF_vect){
	if(timerCountdown>0) timerCountdown--;
	if(timerCountdown == 0) timerCountup++;
	TCA0.SINGLE.INTFLAGS |= TCA_SINGLE_OVF_bm; // must clear the interrupt	
}

ISR(USART3_RXC_vect){
	rx_ready = 1;
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

void handle_rx_ready(){
	
	switch(ui_state){
		case(IDLE):
			printf("At the signal draw your weapon\n");
			printf("Press any button on the board to shoot\n");
			long r = (random() % 3000) + 2000; // random: (2000 - 5000)
			timerCountdown = r;
			timerCountup = 0;
			ui_state = WAITING_TIMER;
			break;
		case(WAITING_TIMER):
			break;
		case(WAITING_INPUT):
			break;
		case(RESET):
			printf("Ready?\n");
			printf("(Press any button to start)\n");
			ui_state = IDLE;
			break;
	}
}

void handle_timer_done(){
	switch(ui_state){
		case(IDLE):
			break;
		case(WAITING_TIMER):
			PORTD.OUTSET = PIN7_bm;
			ui_state = WAITING_INPUT;
			break;
		case(WAITING_INPUT):
			break;
		case(RESET):
			break;
}

void handle_btn_pressed(){
	int result = timerCountup;
	timerCountdown = -1;
	PORTD.OUTCLR = PIN7_bm;
	printf("Your time: %d(ms)\n", result);
	if(result < 1000) printf("Quick draw!\n");
	printf("Play again?\n");
	printf("(Press any button to continue)");;
	ui_state = RESET;
}

void buttonSM(){
	btn_state_t btn_state = RELEASED;
	switch(btn_state){
		case RELEASED:
			if(BTN) btn_state = MAYBE_PUSHED;
			break;
		case MAYBE_PUSHED:
			if(BTN){
				btn_state = PUSHED;
				btnPushed = true;
				handle_btn_pressed();
			}
			else btn_state = RELEASED;
			break;
		case PUSHED:
			if(!BTN) btn_state = MAYBE_RELEASED;
			break;
		case MAYBE_RELEASED:
			if(BTN) btn_state = PUSHED;
			else btn_state = RELEASED;		
			break;
	}
}

void startup(){
	// TODO: Create startup message
	continue;
}