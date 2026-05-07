/*
 * LabExam05.c
 *
 * Created: 4/15/2026 1:43:41 PM
 * Author : samou
 */ 

#define F_CPU 16000000UL

#define EEPROM_FLAG_ADR (uint8_t*)0	// store flag at position 0
#define EEPROM_DATA_ADR (float*)1	// store data at position 1
#define BUFFER_SIZE 16

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include <util/delay.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "lcd_lib.h"
#include "I2C_lib.h"
#include "ADC.h"
#include "DAC.h"
#include "TCA0.h"
#include "SPI.h"
#include "MPC4131.h"
#include "uart.h"
#include "app.h"

#define BTN (!(VPORTB.IN & PIN2_bm))
#define DEBOUNCE_TIME 10

typedef enum {RELEASED, MAYBE_PUSHED, PUSHED, MAYBE_RELEASED
} btn_state_t;

/* FUNCTION PROTOTYPES */
// -- SYSTEM
void init_clock(void);
void buttonSM(void);
void save_current_difference(void);
void show_saved_difference(void);

/* GLOBAL VARIABLES */
volatile uint16_t two_sec_timer = 0;
volatile uint16_t one_sec_counter = 100;
volatile int debounce_timerCount = DEBOUNCE_TIME;
volatile bool btnPushed = false;
volatile bool handled = false;

int main(void)
{
	/* Initializations */
	
	PORTB.DIRCLR = PIN2_bm;
	PORTB.PIN2CTRL = PORT_PULLUPEN_bm;
	init_clock();
	TWI_Host_Initialize();
	LCDinitialize();
	InitTCA0();
	init_ADC();
	InitDAC();
	SPI_Host_Init();
	uart_init(3, 9600, NULL);
	sei();
	
    if (eeprom_read_byte(EEPROM_FLAG_ADR) == 'T') {
        show_saved_difference();
        while (handled) {}
    }

    while (1) {
        if (!handled && one_sec_counter == 0) {
            cli();
            one_sec_counter = 1000;
            sei();

            update_outputs_and_display();
        }

        if (btnPushed && !handled) {
            cli();
            btnPushed = false;
            sei();

            save_current_difference();
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
	if(debounce_timerCount>0)
	{
		debounce_timerCount--;
	}
	else
	{
		buttonSM();
		debounce_timerCount = DEBOUNCE_TIME;
	}
	if(handled){
		if(two_sec_timer>0)
		{
			two_sec_timer--;
		}
		else{
			handled = false;
		}
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

void buttonSM(void)
{
    static btn_state_t btn_state = RELEASED;

    switch (btn_state) {
        case RELEASED:
            if (BTN) btn_state = MAYBE_PUSHED;
            break;

        case MAYBE_PUSHED:
            if (BTN) {
                btn_state = PUSHED;
                btnPushed = true;
            } else {
                btn_state = RELEASED;
            }
            break;

        case PUSHED:
            if (!BTN) btn_state = MAYBE_RELEASED;
            break;

        case MAYBE_RELEASED:
            if (BTN) btn_state = PUSHED;
            else btn_state = RELEASED;
            break;
    }
}

void save_current_difference(void)
{
    float diff = measure_difference();

    eeprom_write_float(EEPROM_DATA_ADR, diff);
    eeprom_write_byte(EEPROM_FLAG_ADR, 'T');

    handled = true;
    two_sec_timer = 2000;
    display_difference(diff);
}

void show_saved_difference(void)
{
    float diff = eeprom_read_float(EEPROM_DATA_ADR);

    handled = true;
    two_sec_timer = 2000;
    display_difference(diff);
}