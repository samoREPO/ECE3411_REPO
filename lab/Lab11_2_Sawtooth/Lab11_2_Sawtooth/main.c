/*
 * Lab11_2_Sawtooth.c
 *
 * Created: 3/28/2026 11:50:13 AM
 * Author : samou
 */ 

// -- MACRO DEFINITIONS -- //
#define F_CPU 16000000UL
#define ADC_MAX 4095
#define PER_MAX 999

#include <avr/io.h>
#include <avr/interrupt.h>

// -- FUNCTION PROTOTYPES -- //
void InitTCA0(void);
void InitTCA1(void);
void init_ADC(void);
void update_period(void);

// -- GLOBAL VARIABLES -- //
volatile uint16_t ramp = 0;

int main(void)
{
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
		update_period();
    }
}

// -- TIMER FUNCITONS --//
void InitTCA0(void)
{
	TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc; // Normal mode
	TCA0.SINGLE.PER = PER_MAX; // Set number of ticks per period
	
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm; // Enable TCA0 Overflow ISR
	
	TCA0.SINGLE.CTRLA |= TCA_SINGLE_CLKSEL_DIV16_gc; // Set Prescalar to 16. each tick is 1us.
	TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm; // enable timer
}

ISR(TCA0_OVF_vect)
{
	if (ramp >= TCA1.SINGLE.PER)
	{
		ramp = 0;
	}
	else
	{
		ramp++;
	}

	TCA1.SINGLE.CMP2BUF = ramp;
	TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
}

// -- PWM FUNCTIONS -- //
void InitTCA1(void)
{
	TCA1.SINGLE.PER = 250; // TOP = 1000 ticks. T=1ms.
	
	TCA1.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm; // enable TCA0
	TCA1.SINGLE.CTRLA |= TCA_SINGLE_CLKSEL_DIV16_gc ; // Set CLK to 16MHz 'div' 16 = 1MHz
	
	TCA1.SINGLE.CTRLB |= TCA_SINGLE_WGMODE_SINGLESLOPE_gc; // enable pwm single slope mode
	TCA1.SINGLE.CTRLB |= TCA_SINGLE_CMP2_bm; // enable WO2.
	PORTMUX.TCAROUTEA |= PORTMUX_TCA1_PORTC_gc; // route WO2 to PORTC (Pin2)
	
	PORTC.DIRSET = PIN6_bm;
	
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

void update_period(void)
{
		uint16_t r = ADC0.RES;   // 0..4095
		uint16_t new_per = ((uint32_t)r * 250) / 4095;
		TCA1.SINGLE.PERBUF = new_per;
		ADC0.COMMAND = ADC_STCONV_bm;
}