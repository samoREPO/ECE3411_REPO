/*
 * Gameboy.c
 *
 * Created: 4/20/2026 2:48:21 PM
 * Author : samou
 */ 

#include <avr/io.h>

#include "button.h"
#include "uart.h"

/* FUNCTION PROTOTYPES */
void init_clock(void);

int main(void)
{
	/* Initializations */
	init_clock();
	uart_init(3,9600,NULL);

	
    while (1) 
    {
		if(BTN_CLEAR)
		{
			printf("btn_clear\n");
		}
		if(BTN_MODE)
		{
			printf("btn_mode\n");
		}
    }
}

/* CLOCK */
void init_clock(void)
{
	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.XOSCHFCTRLA = CLKCTRL_FRQRANGE_16M_gc | CLKCTRL_ENABLE_bm;

	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.MCLKCTRLA = CLKCTRL_CLKSEL_EXTCLK_gc;

	while (!(CLKCTRL.MCLKSTATUS & CLKCTRL_EXTS_bm));
}