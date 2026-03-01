/*
 * LabExam02.c
 *
 * Created: 2/25/2026 1:36:21 PM
 * Author : samou
 */ 

#define F_CPU 16000000UL
#define BTN1 (!(PORTB.IN & PIN2_bm))

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdbool.h>
#include "uart.h"

typedef enum {MODE0, MODE1
} ui_state_t;

// blink functions
void delay_ms_var(uint16_t ms);
void blink(uint8_t frequency, uint8_t position);

//UART functions
void init_clock();
void handle_rx_char(char rx_c);
void handle_frequency_input(uint8_t frequency_update);

// ui functions
void handle_button();

// interrupt off functions
void handle_char(char input);

// blink control variables
uint8_t frequency = 2;
uint8_t position = 3;
uint8_t pin[] = {PIN0_bm, PIN1_bm, PIN2_bm, PIN3_bm, PIN4_bm, PIN5_bm, PIN6_bm, PIN7_bm};

// UART flags
volatile uint8_t rx_ready = 0;
volatile uint8_t rx_byte = 0;
uint8_t frequency_buffer = 0;

// UI variables
volatile ui_state_t ui_state = MODE0;
uint16_t counter = 0; // 2 second counter
bool ui_flag = false;

int main(void)
{
	// LED setup
	PORTD.DIRSET = 0xFF;
	PORTD.OUTCLR = 0xFF;
	
	// button setup
	PORTB.DIRCLR = PIN2_bm;
	PORTB.PIN2CTRL |= PORT_PULLUPEN_bm;
	
	// ui setup
	uint8_t button1Handled = 0; //flag that button1 was pressed
	
	// UART setup
	init_clock();
	uart_init(3, 19200, NULL);
	sei();
	
	USART3.CTRLA |= USART_RXCIE_bm; // enable UART RX interrupt
	
    /* Replace with your application code */
    while (1) 
    {
		blink(frequency, position);
		
		if(BTN1){
			if(!button1Handled){
				button1Handled = 1;
				ui_flag = true;
			}
		} else
		{
			if(ui_flag){
				ui_flag = false;
				handle_button();
			}
			button1Handled = 0;
		}
		
		switch(ui_state){
			case(MODE0):
				if(rx_ready){
					cli();
					if(rx_ready) rx_ready = 0;
					handle_rx_char(rx_byte);
					sei();
				}
				break;
			case(MODE1):
				counter += 1000/ frequency;
				if(counter >= 2000){
					printf("enter something\n");
					char input;
					scanf("%c", &input);
					handle_char(input);
					counter = 0;
				}
				break;
			default: 
				break;
		}
    }
	
}

/************************* blink functions************************/

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

//----------------- UART functions-----------------------------
void init_clock() {

	CPU_CCP = CCP_IOREG_gc;

	CLKCTRL.XOSCHFCTRLA =  CLKCTRL_FRQRANGE_16M_gc | CLKCTRL_ENABLE_bm;

	CPU_CCP = CCP_IOREG_gc;

	CLKCTRL.MCLKCTRLA = CLKCTRL_CLKSEL_EXTCLK_gc;

	while(!(CLKCTRL.MCLKSTATUS & CLKCTRL_EXTS_bm));

}

ISR(USART3_RXC_vect){
	rx_byte = USART3.RXDATAL;
	rx_ready = 1;
}

void handle_rx_char(char rx_c){
	if(rx_c >= '0' && rx_c <= '9') handle_frequency_input(rx_c - '0');
	switch(rx_c){
		case 'w':
			frequency++;
			printf("frequency: %d\n", frequency);
			break;
			
		case 'a':
			if(position > 0) position--;
			break;
		
		case 's':
			if(frequency > 1){ frequency --;}
			printf("frequency: %d\n", frequency);
			break;
		
		case 'd':
			if(position < 7) position++;
			break;
			
		case '\r':
			if(frequency_buffer > 0){frequency = frequency_buffer;}
			frequency_buffer = 0;
			printf("frequency: %d\n", frequency);
			break;
		default:
			break;
		
	}
}

void handle_frequency_input(uint8_t frequency_update){
	if(frequency_buffer == 0){frequency_buffer = frequency_update;}
	else{frequency_buffer = (frequency_buffer*10) + frequency_update;}
}

void handle_button(){
	if(ui_state == MODE0) {
		ui_state = MODE1;
		USART3.CTRLA &= 0x7F;
		counter = 0;
	}
	else if (ui_state == MODE1) {
		ui_state = MODE0;
		USART3.CTRLA |= 0x80;
	}
}

void handle_char(char input){
	switch(input){
		case 'w':
		frequency++;
		printf("frequency: %d\n", frequency);
		break;
		
		case 'a':
		if(position > 0) position--;
		break;
		
		case 's':
		if(frequency > 1){ frequency --;}
		printf("frequency: %d\n", frequency);
		break;
		
		case 'd':
		if(position < 7) position++;
		break;
		
		default:
		break;
	}
}