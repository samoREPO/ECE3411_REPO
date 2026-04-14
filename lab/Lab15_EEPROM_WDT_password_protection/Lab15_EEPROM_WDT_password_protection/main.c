/*
 * Lab15_EEPROM_WDT_password_protection.c
 *
 * Created: 4/13/2026 9:01:38 AM
 * Author : samou
 */ 

#define F_CPU 16000000UL

#define EEPROM_FLAG_ADR (uint8_t*)0	// store flag at position 0
#define EEPROM_DATA_ADR (uint8_t*)1	// store data at position 1
#define BUFFER_SIZE 16

#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "uart.h"

/* FUNCTION PROTOTYPES */

// -- LED
void init_led(void);

// -- CLOCK
void init_clock(void);

// -- UART
void InitTCA0(void);

// -- WDT
void init_WDT (void);
void generate_random_string(char* , int );
void write_random_password(void);
void write_user_password(void);
bool validate_password(void);

/* GLOABAL VARIABLES */
// -- EEPROM
volatile char read_buffer[16];
volatile char write_buffer[16];
volatile bool password_match = false;


int main(void)
{
	/* Initializations */
	// -- LED
	init_led();
	
	// -- UART
	init_clock();
	uart_init(3,9600,NULL);
	InitTCA0();
	sei();
	
	// -- WDT
	init_WDT();

	if(eeprom_read_byte(EEPROM_FLAG_ADR) == 'T')
	{
		wdt_enable(WDT_PERIOD_8KCLK_gc);
		while(!password_match)
		{
			password_match = validate_password();
		}
		wdt_disable();
	}
	else
	{
		write_user_password();
	}

    while (1) 
    {
		PORTD.OUTSET = PIN7_bm;	
    }
}

/* FUNCTION DEFINITIONS */
// -- LED
void init_led(void)
{
	PORTD.DIRSET = PIN7_bm;
	PORTD.OUTCLR = PIN7_bm;
}

// -- CLOCK 
void init_clock(void)
{

	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.XOSCHFCTRLA =  CLKCTRL_FRQRANGE_16M_gc | CLKCTRL_ENABLE_bm;
	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.MCLKCTRLA = CLKCTRL_CLKSEL_EXTCLK_gc;

	while(!(CLKCTRL.MCLKSTATUS & CLKCTRL_EXTS_bm));

}

// -- TIMERS
void InitTCA0(void)
{
	TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc; // Normal mode
	TCA0.SINGLE.PER = 999; // Set number of ticks per period
	
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm; // Enable TCA0 Overflow ISR
	
	TCA0.SINGLE.CTRLA |= TCA_SINGLE_CLKSEL_DIV16_gc; // Set Prescalar to 16. each tick is 1us.
	TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm; // enable timer
}

ISR(TCA0_OVF_vect)
{
	TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
}

// -- WDT
void init_WDT (void)
{
	

	
	// The following lines ensure that the watchdog turns off after reset
	wdt_reset();	// Reset Watchdog Timer
	CPU_CCP = CCP_IOREG_gc;
	WDT.CTRLA = 0;	// Disable Watchdog
	if (RSTCTRL.RSTFR & RSTCTRL_WDRF_bm)
	{
		write_random_password();
	}
	RSTCTRL.RSTFR = RSTCTRL_WDRF_bm; // Turn off Watchdog Reset Flag	
}


// -- EEPROM

void generate_random_string(char *str, int length)
{
	const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

	if (length > 0)
	{
		for (int n = 0; n < length - 1; n++)
		{
			int key = rand() % (sizeof(charset) - 1);
			str[n] = charset[key];
		}
		str[length - 1] = '\0';
	}
}

void write_random_password(void)
{
	char rand_password[BUFFER_SIZE];
	generate_random_string(rand_password, BUFFER_SIZE);
	eeprom_write_block(rand_password, EEPROM_DATA_ADR, BUFFER_SIZE);
	eeprom_write_byte(EEPROM_FLAG_ADR, 'T');
}

void write_user_password(void)
{
	char user_password[BUFFER_SIZE];
	fprintf(stdout, "Enter new password\n\r");
	fgets(user_password, BUFFER_SIZE, stdin);
	eeprom_write_block(user_password, EEPROM_DATA_ADR, BUFFER_SIZE);
	eeprom_write_byte(EEPROM_FLAG_ADR, 'T');
}

bool validate_password(void)
{
	char user_password[BUFFER_SIZE];
	fprintf(stdout, "Enter correct password\n\r");
	fgets(user_password, BUFFER_SIZE, stdin);
	
	char stored_password[BUFFER_SIZE];
	eeprom_read_block(stored_password, EEPROM_DATA_ADR, BUFFER_SIZE);
	
	if(strcmp(user_password, stored_password) == 0) return true;
	else return false;
}