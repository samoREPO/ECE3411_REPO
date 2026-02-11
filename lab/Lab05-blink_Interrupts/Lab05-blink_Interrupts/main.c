/*
 * Lab05-blink_Interrupts.c
 *
 * Created: 2/10/2026 7:45:24 PM
 * Author : samou
 */ 


#define F_CPU 16000000UL
#include "uart.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>

#define DIRSET_gm 0xFF
#define OUTCLR_gm 0xFF

typedef enum { WAIT_SELECT, WAIT_FREQUENCY, WAIT_POSITION} ui_state_t;
volatile ui_state_t ui_state = WAIT_SELECT;

volatile uint8_t rx_ready = 0;
volatile uint8_t rx_byte = 0;
volatile char selected = 0;

volatile uint8_t frequency = 2;
volatile uint8_t position = 2;
uint8_t pin[] = {PIN0_bm, PIN1_bm, PIN2_bm, PIN3_bm, PIN4_bm, PIN5_bm, PIN6_bm, PIN7_bm};

void init_clock();
void delay_ms_var(uint16_t ms);
void blink(uint8_t frequency, uint8_t position);
void handle_rx_char(char c);



int main(void)
{
	PORTD.DIRSET = DIRSET_gm;
	PORTD.OUTCLR = OUTCLR_gm;
	
	init_clock();	
	uart_init(3, 9600, NULL);
		
	USART3.CTRLA |= USART_RXCIE_bm; // enable UART TX interrupt
	sei();

    while (1) 
    {
		
	if(rx_ready){
		cli();
		if(rx_ready) rx_ready = 0;
		sei();

		handle_rx_char(rx_byte);
	}

	blink(frequency, position);

    }

}

ISR(USART3_RXC_vect){
	rx_byte = USART3.RXDATAL;
	rx_ready = 1;
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

// 50% duty cycle blink function
void blink(uint8_t frequency, uint8_t position){
	uint16_t half_period_ms = 1000 / (2*frequency);     // recall T = 1/f
	PORTD.OUTTGL = pin[position];
	delay_ms_var(half_period_ms);
	PORTD.OUTTGL = pin[position];
	delay_ms_var(half_period_ms);
}

void handle_rx_char(char c){
	if (c >= 'A' && c <= 'Z') c += ('a' - 'A');

	switch(ui_state){
		case WAIT_SELECT:
			if(c == 'f'){
				ui_state = WAIT_FREQUENCY;
				printf("Press a desired frequency(1-9):");
			}
			else if(c == 'p'){
				ui_state = WAIT_POSITION;
				printf("Press a desired position(0-7):");
			}
			else{
				printf("INVALID SELECTION\n");
				printf("Press F/f to change frequency or P/p to change position\n");
			}
			break;
		case WAIT_FREQUENCY:
			if(c >= '1' && c <= '9'){
				uint8_t d = (uint8_t)(c - '0');
				frequency = d;
			}
			ui_state = WAIT_SELECT;
			printf("Press F/f to change frequency or P/p to change position\n");
			break;
		case WAIT_POSITION:
			if(c >= '0' && c <= '7'){
				uint8_t d = (uint8_t)(c - '0');
				position = d;
			}
			ui_state = WAIT_SELECT;
			printf("Press F/f to change frequency or P/p to change position\n");
			break;
	}
}
