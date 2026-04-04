/*
 * Lab13-DAC_Sine.c
 *
 * Created: 4/1/2026 8:04:54 AM
 * Author : samou
 */ 

#define F_CPU 16000000 // CPUclk = 16 MHz
#include <avr/io.h>
#include <stdbool.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "lcd_lib.h"
#include "I2C_lib.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/interrupt.h>

#define DEBOUNCE_TIME 10000 // 10ms debounce time
#define BTN (!(PORTB.IN & PIN2_bm))
#define UPDATE_TIME 100
#define PER_MAX 1000000/64

typedef enum {RELEASED, MAYBE_PUSHED, PUSHED, MAYBE_RELEASED
} btn_state_t;

// -- GLOBAL VARIABLES
volatile int debounce_timerCount = DEBOUNCE_TIME;
volatile bool btnPushed = false;

bool frequency_high = false;
uint8_t base_frequency = 10;
uint8_t curr_frequency = 10;
float curr_amplitude_percent = 1.0;
volatile uint8_t update_timer = UPDATE_TIME;
volatile uint8_t index_sine_table = 0;

// Sine wave lookup table(64 samples, 10-bit resolution)
const uint16_t sine_table[64] PROGMEM = {
	512, 562, 612, 660, 707, 752, 794, 833,
	868, 900, 927, 950, 968, 982, 991, 995,
	995, 991, 982, 968, 950, 927, 900, 868,
	833, 794, 752, 707, 660, 612, 562, 512,
	461, 411, 363, 316, 271, 229, 190, 155,
	123, 95, 72, 53, 39, 30, 25, 24,
	28, 37, 50, 68, 90, 116, 146, 180,
	218, 259, 303, 350, 399, 449, 500, 551
};

// -- FUNCTION PROTOTYPES
// -- -- BUTTON PROTOTYPES
void init_button(void);
void buttonSM(void);
void handle_btn_push(void);
// -- -- TCA PROTOTYPES
void InitTCA0(void);
void InitTCA1(void);
// -- -- DAC PROTOTYPES
void InitDAC(void);
void DAC_ouput(void);
// -- ADC PROTOTYPES
void init_ADC(void);
// -- I2C PROTOTYPES
void init_clock(void);
// -- MISC PROTOTYPES
void update_Amplitude(void);
void update_Frequency(void);

int main(void)
{
    // Initializations:
    // -- LCD
    init_clock();
    TWI_Host_Initialize();
    LCDinitialize();
	
	// -- BUTTON
	init_button();
	
	// -- TIMERS
	InitTCA0();
	InitTCA1();
	
	// -- DAC
	InitDAC();
	
	// -- ADC
	init_ADC();
	
	
	sei();
	
    while (1) 
    {
		if(btnPushed){
			cli();
			btnPushed = false;
			sei();
			handle_btn_push();
		}
		if(update_timer == 0)
		{
			cli();
			update_timer = UPDATE_TIME;
			sei();
			update_Frequency();
			update_Amplitude();
		}
    }
}

// -- BUTTON FUNCTIONS
void init_button(void)
{
	PORTB.DIRCLR = PIN2_bm;
	PORTB.PIN2CTRL |= PORT_PULLUPEN_bm;
}
void buttonSM(void)
{
	static btn_state_t btn_state = RELEASED;
	switch(btn_state){
		case RELEASED:
		if(BTN) btn_state = MAYBE_PUSHED;
		break;
		case MAYBE_PUSHED:
		if(BTN){
			btn_state = PUSHED;
			btnPushed = true;
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

void handle_btn_push(void)
{
	if(frequency_high)
	{
		base_frequency = 10;
	}
	else
	{
		base_frequency = 100;
	}
	
}

// -- TIMER FUNCTIONS
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
	if(update_timer>0)
	{
		update_timer--;
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
	
	TCA0.SINGLE.INTFLAGS |= TCA_SINGLE_OVF_bm; // must clear the interrupt
}

void InitTCA1(void)
{
	TCA1.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc; // Normal mode
	TCA1.SINGLE.PER = PER_MAX / base_frequency; // Set number of ticks per period
	
	TCA1.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm; // Enable TCA0 Overflow ISR
	
	TCA1.SINGLE.CTRLA |= TCA_SINGLE_CLKSEL_DIV16_gc; // Set Prescalar to 16. each tick is 1us.
	TCA1.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm; // enable timer
}

ISR(TCA1_OVF_vect)
{
	if(index_sine_table < 64)
	{
		index_sine_table++;
	}
	else
	{
		index_sine_table = 0;
	}
	DAC_ouput();
	TCA1.SINGLE.INTFLAGS |= TCA_SINGLE_OVF_bm; // must clear the interrupt
}

// -- DAC FUNCTIONS
void InitDAC(void)
{
	DAC0.CTRLA = DAC_ENABLE_bm | DAC_OUTEN_bm;
	// Set DAC reference to VDD
	VREF.DAC0REF = VREF_REFSEL_VDD_gc;

}

void DAC_ouput(void)
{
	uint16_t sample = pgm_read_word(&sine_table[index_sine_table]);
	sample = (int)((float) (curr_amplitude_percent * sample));
	DAC0.DATAL = sample & 0x03; // write low 2 bits
	DAC0.DATAH =(sample >> 2) & 0xFF; 
}

// -- ADC FUNCTIONS -- //
void init_ADC(void){
	ADC0.MUXPOS = ADC_MUXPOS_AIN8_gc; // Input voltage at AIN8 PORTE0
	ADC0.CTRLC = ADC_PRESC_DIV16_gc; // Set CLK_ADC to 1MHz (valid range from 125kHz to 2MHz)
	ADC0.CTRLD = ADC_INITDLY_DLY16_gc; // Delay 16 CLK_ADC cycles - 16 us
	VREF.ADC0REF = VREF_REFSEL_VDD_gc; // VDD as reference
	ADC0.CTRLA |= ADC_ENABLE_bm; // enables the ADC circuitry and left adjusted and 12-bit (3.3/2^12=0.8mV resolution
	ADC0.COMMAND = ADC_STCONV_bm; // start first A to D conversion
	
	while(ADC0.COMMAND & ADC_STCONV_bm);
}

// -- I2C FUNCTIONS
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

// -- MISC FUNCTIONS
void update_Amplitude(void)
{
	ADC0.MUXPOS = ADC_MUXPOS_AIN8_gc; // Input voltage at AIN8 PORTE0
	ADC0.COMMAND = ADC_STCONV_bm; // start next A to D conversion
	loop_until_bit_is_clear(ADC0.COMMAND, ADC_STCONV_bp);
	
	uint16_t r = ADC0.RES;
	
	curr_amplitude_percent = ((float)r/4096.0f);
	
	//TODO update the sample values
	
	char AmplitudeBuffer[6];
	dtostrf(curr_amplitude_percent * 3.3f, 4, 2, AmplitudeBuffer);
	
	char LCDout [20] = "A: ";
	strcat(LCDout, AmplitudeBuffer);
	
	LCDgotoXY(0,0);
	LCDstring(LCDout);
	
	//TODO: actually scale ROM TABLE
}

void update_Frequency(void)
{
	ADC0.MUXPOS = ADC_MUXPOS_AIN9_gc; // Input voltage at AIN8 PORTE1
	ADC0.COMMAND = ADC_STCONV_bm; // start next A to D conversion
	loop_until_bit_is_clear(ADC0.COMMAND, ADC_STCONV_bp);

	uint16_t r = ADC0.RES;

	curr_frequency = (1 + 9*((float)r/4096.0f)) * base_frequency;

	char FrequencyBuffer[6];
	dtostrf(curr_frequency, 4, 2, FrequencyBuffer);

	char LCDout [20] = "f: ";
	strcat(LCDout, FrequencyBuffer);

	LCDgotoXY(0,1);
	LCDstring(LCDout);
}