/*
 * Lab11_1_PWM_BrightnessControl.c
 *
 * Created: 3/20/2026 2:09:56 PM
 * Author : samou
 */ 

// -- MACRO DEFINITIONS -- //
#define F_CPU 16000000UL
#define ADC_MAX 4095.0
#define ONE_SEC 1000

// -- INCLUDES -- //
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "lcd_lib.h"
#include "I2C_lib.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// -- FUNCTION PROTOTYPES -- //
void InitTCA0(void);
void InitTCA1(void);
void init_ADC(void);
void init_clock(void);
void update_DutyCycle(void);

// -- GLOBAL VARIABLES -- //
volatile uint16_t one_sec_counter = ONE_SEC;

int main(void)
{
    // Initializations:
	// -- LCD
    init_clock();
    TWI_Host_Initialize();
    LCDinitialize();
	// -- TIMER
	InitTCA0();
	// -- PWM
	InitTCA1();
	// -- ADC
	init_ADC();
	
	//enable interrupts
	sei();
	 
    while (1) 
    {
		if(one_sec_counter == 0){
			cli();
			one_sec_counter = ONE_SEC;
			sei();
			update_DutyCycle();
		}
    }
}

// -- TIMER FUNCITONS --//
void InitTCA0()
{
	TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc; // Normal mode
	TCA0.SINGLE.PER = 999; // Set number of ticks per period
	
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm; // Enable TCA0 Overflow ISR
	
	TCA0.SINGLE.CTRLA |= TCA_SINGLE_CLKSEL_DIV16_gc; // Set Prescalar to 16. each tick is 1us.
	TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm; // enable timer
}

ISR(TCA0_OVF_vect)
{
	if(one_sec_counter>0){one_sec_counter--;}
	TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
}

// -- PWM FUNCTIONS -- //
void InitTCA1(void)
{
	TCA1.SINGLE.PER = 999; // TOP = 1000 ticks. T=1ms.
	
	TCA1.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm; // enable TCA0
	TCA1.SINGLE.CTRLA |= TCA_SINGLE_CLKSEL_DIV16_gc ; // Set CLK to 16MHz 'div' 16 = 1MHz
	
	TCA1.SINGLE.CTRLB |= TCA_SINGLE_WGMODE_SINGLESLOPE_gc; // enable pwm single slope mode
	TCA1.SINGLE.CTRLB |= TCA_SINGLE_CMP2_bm; // enable WO2.
	PORTMUX.TCAROUTEA |= PORTMUX_TCA1_PORTB_gc; // route WO2 to PORTB (Pin2)
	
	PORTB.DIRSET = PIN2_bm;
	
	TCA1.SINGLE.INTCTRL;
}

// -- ADC FUNCTIONS -- //
void init_ADC(void)
{
	ADC0.MUXPOS = ADC_MUXPOS_AIN8_gc; // Input voltage at AIN8 PORTE0
	ADC0.CTRLC = ADC_PRESC_DIV16_gc; // Set CLK_ADC to 1MHz (valid range from 125kHz to 2MHz)
	ADC0.CTRLD = ADC_INITDLY_DLY16_gc; // Delay 16 CLK_ADC cycles - 16 us
	VREF.ADC0REF = VREF_REFSEL_VDD_gc; // VDD as reference
	ADC0.CTRLA |= ADC_ENABLE_bm; // enables the ADC circuitry and left adjusted and 12-bit (3.3/2^12=0.8mV resolution
	ADC0.COMMAND = ADC_STCONV_bm; // start first A to D conversion
	
	while(ADC0.COMMAND & ADC_STCONV_bm);
}

void update_DutyCycle(void)
{
	uint16_t r = ADC0.RES; // Read ADC
	ADC0.COMMAND = ADC_STCONV_bm; // start next A to D conversion
	
	float duty_cycle = (float) r / ADC_MAX;
	float duty_cycle_percent = 100 * duty_cycle;
	
	TCA1.SINGLE.CMP2 = 1000 * duty_cycle - 1;
	
	char DutyCycleBuffer[6];
	dtostrf(duty_cycle_percent, 2, 2, DutyCycleBuffer);
	
	char LCDout [20] = "Duty Cycle: ";
	strcat(LCDout, DutyCycleBuffer);
	
	LCDgotoXY(0,0);
	LCDstring(LCDout);
}

// -- I2C FUNCTIONS -- //
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




void update_period(void)
{
	uint16_t r = ADC0.RES;   // 0..4095
	uint16_t new_per = ((uint32_t)r * PER_MAX) / 4095;
	TCA1.SINGLE.PERBUF = new_per;
	ADC0.COMMAND = ADC_STCONV_bm;
}