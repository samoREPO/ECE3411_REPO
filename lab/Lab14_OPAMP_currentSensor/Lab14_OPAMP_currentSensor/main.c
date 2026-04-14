/*
 * Lab14_OPAMP_currentSensor.c
 *
 * Created: 4/8/2026 11:44:13 AM
 * Author : samou
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <util/delay.h>
#include <string.h>
#include "lcd_lib.h"
#include "I2C_lib.h"

void init_ADC(void);
void init_clock(void);
void InitTCA0(void);
void InitOpAmp_012(void);
void read_ADC_then_print(void);

volatile uint16_t one_sec_counter = 1000;

int main(void)
{
	// LCD setup
	init_clock();
    TWI_Host_Initialize();
    LCDinitialize();
	
	
	// ADC setup
	init_ADC();
	
	// OPAMP setup
	InitOpAmp_012();
	
	InitTCA0();
	
	// enable interrupts
	sei();
	
    while (1) 
    {
		if(one_sec_counter == 0)
		{
			cli();
			one_sec_counter = 1000;
			sei();
			read_ADC_then_print();
		}
    }
}

/* ~~~~~~~~~~~~~~~~~~~~~ OP-AMP FUNCTIONS ~~~~~~~~~~~~~~~~~~~ */

void InitOpAmp_012(void)
{
	/* Configure the timebase */
	OPAMP.TIMEBASE = (ceil(F_CPU / 1e6)-1); 
	
	// OpAmp 0
	OPAMP.OP0INMUX = OPAMP_OP0INMUX_MUXPOS_INP_gc | OPAMP_OP0INMUX_MUXNEG_OUT_gc; // Connect NINV to input and INV to Output
		
	OPAMP.OP0RESMUX = OPAMP_OP0RESMUX_MUXBOT_GND_gc |
	OPAMP_OP0RESMUX_MUXWIP_WIP3_gc | // unity gain R1 =8, R2 = 8
	OPAMP_OP0RESMUX_MUXTOP_OUT_gc;
		
	OPAMP.OP0CTRLA = OPAMP_OP0CTRLA_OUTMODE_NORMAL_gc | OPAMP_ALWAYSON_bm;
		
	OPAMP.OP0SETTLE = 0x7F;
		
	// OpAmp 1
	OPAMP.OP1INMUX = OPAMP_OP1INMUX_MUXPOS_INP_gc | OPAMP_OP1INMUX_MUXNEG_OUT_gc; // Connect NINV to input and INV to Output
		
	OPAMP.OP1RESMUX = OPAMP_OP1RESMUX_MUXBOT_OFF_gc |
	OPAMP_OP1RESMUX_MUXWIP_WIP0_gc | // R1= 15, R2 = 1
	OPAMP_OP1RESMUX_MUXTOP_OFF_gc;
		
	OPAMP.OP1CTRLA = OPAMP_OP1CTRLA_OUTMODE_NORMAL_gc | OPAMP_ALWAYSON_bm;
		
	OPAMP.OP0SETTLE = 0x7F;
	
	// OpAmp 2
	OPAMP.OP2INMUX = OPAMP_OP2INMUX_MUXPOS_LINKWIP_gc | OPAMP_OP2INMUX_MUXNEG_WIP_gc; // Connect NINV to input and INV to Output
		
	OPAMP.OP2RESMUX = OPAMP_OP2RESMUX_MUXBOT_LINKOUT_gc |
	OPAMP_OP2RESMUX_MUXWIP_WIP3_gc | // unity gain R1=8, R2=8
	OPAMP_OP2RESMUX_MUXTOP_OUT_gc;
		
	OPAMP.OP2CTRLA = OPAMP_OP2CTRLA_OUTMODE_NORMAL_gc | OPAMP_ALWAYSON_bm;
		
	OPAMP.OP2SETTLE = 0x7F;
	
	OPAMP.CTRLA = OPAMP_ENABLE_bm;
	
	while(!(OPAMP.OP0STATUS & OPAMP_SETTLED_bm));
	while(!(OPAMP.OP1STATUS & OPAMP_SETTLED_bm));
	while(!(OPAMP.OP2STATUS & OPAMP_SETTLED_bm));
	
	PORTD.PIN1CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTD.PIN3CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTD.PIN4CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTD.PIN7CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTE.PIN1CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTE.PIN3CTRL = PORT_ISC_INPUT_DISABLE_gc;
	
	PORTE.DIRSET = PIN2_bm;

}

//--------------------- ADC functions-----------------------//
void init_ADC(void){
	ADC0.MUXPOS = ADC_MUXPOS_AIN8_gc; // Input voltage at AIN8 PORTE0
	ADC0.CTRLC = ADC_PRESC_DIV16_gc; // Set CLK_ADC to 1MHz (valid range from 125kHz to 2MHz)
	ADC0.CTRLD = ADC_INITDLY_DLY16_gc; // Delay 16 CLK_ADC cycles - 16 us
	VREF.ADC0REF = VREF_REFSEL_VDD_gc; // VDD as reference
	ADC0.CTRLA |= ADC_ENABLE_bm; // enables the ADC circuitry and left adjusted and 12-bit (3.3/2^12=0.8mV resolution
	ADC0.COMMAND = ADC_STCONV_bm; // start first A to D conversion
	
	while(ADC0.COMMAND & ADC_STCONV_bm);
}

void read_ADC_then_print(void){ //TODO: Edit this function for the ADC in this assignment
	uint16_t r = ADC0.RES; // TODO: check if i can do this instead of RESL then RESH
	float Current = ((float)r/4095.0f) * 50.0f; // 5.0*(RES/(2^12))/0.1
	
	ADC0.COMMAND = ADC_STCONV_bm; // start next A to D conversion
	
	char CurrentBuffer[6];
	dtostrf(Current, 4, 2, CurrentBuffer);

	char LCDout [20] = "Cur: ";
	strcat(LCDout, CurrentBuffer);
	
	LCDgotoXY(0,0);
	LCDstring(LCDout);
}

/* ~~~~~~~~~~~~~~~~~~~~~~~ I2C FUNCTIONS ~~~~~~~~~~~~~~~~~~~*/
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

// -- TIMER FUNCITONS --//
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
	if(one_sec_counter > 0)
	{
		one_sec_counter--;
	}
	TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
}