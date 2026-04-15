/*
 * LabExam5.c
 *
 * Created: 4/15/2026 8:57:26 AM
 * Author : samou
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include <util/delay.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "lcd_lib.h"
#include "I2C_lib.h"
#include "ADC.h"
#include "TCA0.h"
#include "SPI.h"
#include "AnalogComparator.h"
#include "MPC4131.h"
#include "uart.h"
#include "app.h"

/* FUNCTION PROTOTYPES */
// -- SYSTEM
void init_clock(void);

/* GLOBAL VARIABLES */
volatile uint16_t one_sec_counter = 1000;

int main(void)
{
    /* Initializations */
	init_clock();
	TWI_Host_Initialize();
	LCDinitialize();
	InitTCA0();
	init_AC();
	init_ADC();
	SPI_Host_Init();
	uart_init(3, 9600, NULL);
	sei();
	
    while (1) 
    {
		uint8_t do_update = 0;
		cli();
		if (one_sec_counter == 0) {
			one_sec_counter = 1000;
			do_update = 1;
		}
		sei();

		if (do_update) {
			update_and_display_voltages();
		}
    }
}

/* FUNCTION DEFINITIONS */

ISR(TCA0_OVF_vect)
{
	if(one_sec_counter > 0)
	{
		one_sec_counter--;
	}
	TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
}

void init_clock(void)
{
	//	set internal oscillator to 16MHz
	//	CPU_CCP = CCP_IOREG_gc;
	//	CLKCTRL.OSCHFCTRLA = CLKCTRL_FRQSEL_16M_gc;

	//	use external 16MHz crystal
	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.XOSCHFCTRLA = CLKCTRL_FRQRANGE_16M_gc | CLKCTRL_ENABLE_bm;
	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.MCLKCTRLA = CLKCTRL_CLKSEL_EXTCLK_gc;
	while(!(CLKCTRL.MCLKSTATUS & CLKCTRL_EXTS_bm));
}
