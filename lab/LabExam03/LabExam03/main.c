/*
 * LabExam03.c
 *
 * Created: 3/23/2026 9:17:18 AM
 * Author : samou
 */ 
#define F_CPU 16000000UL // CPUclk = 16MHz
#define DEBOUNCE_TIME 10
#define BTN1 (!(PORTB.IN & PIN2_bm))

#include "uart.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// -- UART Prototypes -- //
void init_clock(void);
void InitTCA0(void);

// -- ADC Prototypes -- //
void init_ADC(void);
void read_temperature(void);
void read_voltage(void);
void print_temperature(float);
void print_voltage(float);
void buttonSM(void);

// -- LED Prototypes -- //
void init_LEDs(void);
void LED_ctrl(float voltage);

typedef enum {RELEASED, MAYBE_PUSHED, PUSHED, MAYBE_RELEASED
} btn_state_t;

uint16_t second_counter = 1000;
volatile int debounce_timerCount = DEBOUNCE_TIME;
volatile uint16_t sec_counter = 1000;
volatile uint16_t twenty_ms_counter = 20;
volatile float accVoltage = 0;
volatile float accTemp = 0;
volatile bool btnPushed = false;
volatile float num_samples = 0;

int main(void)
{
	// LED setup
	init_LEDs();

	// UART setup
	init_clock();
	uart_init(3,9600,NULL);
	InitTCA0();
	sei();
	
	// ADC setup
	init_ADC();

    while (1) 
    {
			if(twenty_ms_counter == 0)
			{
				cli();
				twenty_ms_counter = 20;
				num_samples++;
				read_temperature();
				read_voltage();
				sei();
			}
			if(sec_counter == 0){
				float tmp1 = accTemp;
				float tmp2 = accVoltage;
				float tmp3 = num_samples;
				cli();
				accTemp = 0;
				accVoltage = 0;
				num_samples = 0;
				sec_counter = second_counter;
				sei();
				print_temperature((float)(tmp1/tmp3));
				print_voltage((float)(tmp2/tmp3));
			}
			if(btnPushed){
				cli();
				btnPushed = false;
				if(second_counter == 1000)
				{
					second_counter = 2000;
				}
				else if (second_counter == 2000)
				{
					second_counter = 1000;
				}
				sei();
			}
    }
}

// ---------------- LED Functions ----------- //
void init_LEDs(void)
{
	PORTD.DIRSET = (PIN7_bm | PIN6_bm | PIN5_bm | PIN4_bm);
	PORTD.OUTCLR = (PIN7_bm | PIN6_bm | PIN5_bm | PIN4_bm);
}

void LED_ctrl(float voltage)
{
	PORTD.OUTCLR = 0xFF;
	
	if (voltage > 0 && voltage <=0.66) PORTD.OUTCLR = 0xFF;
	else if (voltage > 0.66 && voltage <= 1.33) PORTD.OUTSET = PIN4_bm;
	else if (voltage > 1.33 && voltage <= 2) PORTD.OUTSET = PIN5_bm;
	else if (voltage > 2 && voltage <= 2.66) PORTD.OUTSET = PIN6_bm;
	else if (voltage > 2.66) PORTD.OUTSET = PIN7_bm;
	else PORTD.OUTCLR = 0xFF;
}
//----------------------- UART Functions ------------------------//
void init_clock(void) {

	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.XOSCHFCTRLA =  CLKCTRL_FRQRANGE_16M_gc | CLKCTRL_ENABLE_bm;
	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.MCLKCTRLA = CLKCTRL_CLKSEL_EXTCLK_gc;

	while(!(CLKCTRL.MCLKSTATUS & CLKCTRL_EXTS_bm));

}

void InitTCA0(void){
	TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc; // Normal mode
	TCA0.SINGLE.PER = 1999; // Set number of ticks per period
	
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm; // Enable TCA0 Overflow ISR
	
	TCA0.SINGLE.CTRLA |= TCA_SINGLE_CLKSEL_DIV16_gc; // Set Prescalar to 16. each tick is 1us.
	TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm; // enable timer
}

ISR(TCA0_OVF_vect){
	if(sec_counter>0)
	{
		sec_counter-=2;
	}
	if(twenty_ms_counter>0)
	{
		twenty_ms_counter-=2;
	}
	if (debounce_timerCount >0){
		debounce_timerCount-=2;
	}
	else{
		buttonSM();
		debounce_timerCount = DEBOUNCE_TIME;
	}
	TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
}

//--------------------- ADC functions-----------------------//
void init_ADC(void){
	//ADC0.MUXPOS = ADC_MUXPOS_AIN2_gc; // Input voltage at AIN8 PORTE0
	ADC0.CTRLC = ADC_PRESC_DIV16_gc; // Set CLK_ADC to 1MHz (valid range from 125kHz to 2MHz)
	ADC0.CTRLD = ADC_INITDLY_DLY16_gc; // Delay 16 CLK_ADC cycles - 16 us
	VREF.ADC0REF = VREF_REFSEL_VDD_gc; // VDD as reference
	ADC0.CTRLA |= ADC_ENABLE_bm; // enables the ADC circuitry and left adjusted and 12-bit (3.3/2^12=0.8mV resolution
	ADC0.COMMAND = ADC_STCONV_bm; // start first A to D conversion
	
	while(ADC0.COMMAND & ADC_STCONV_bm);
}

void read_temperature(void)
{
	ADC0.MUXPOS = ADC_MUXPOS_AIN21_gc; // Input voltage at AIN21 PORTF5
	ADC0.COMMAND = ADC_STCONV_bm; // start next A to D conversion

	loop_until_bit_is_clear(ADC0.COMMAND, ADC_STCONV_bp);
			
	uint16_t r = ADC0.RES; // read input
	float Voltage = ((float)r/4096.0f) * 3.3f; // 3.3*(RES/(2^12))
	accTemp += Voltage;

}

void read_voltage(void)
{
	ADC0.MUXPOS = ADC_MUXPOS_AIN2_gc; // Input voltage at AIN2 PORTE0
	
	ADC0.COMMAND = ADC_STCONV_bm; // start next A to D conversion

	loop_until_bit_is_clear(ADC0.COMMAND, ADC_STCONV_bp);

	uint16_t r = ADC0.RES;
	float Voltage = ((float)r/4096.0f) * 3.3f; // 3.3*(RES/(2^12))

	LED_ctrl(Voltage); // Turn on the appropriate LEDs

	accVoltage += Voltage;
}

void print_temperature(float avg_Voltage)
{	
	
	float tempK = 100.0 * ((float) avg_Voltage);
	float tempC = -273.0 + tempK;
	float tempF = (1.8 * tempC) + 32.0;
		
	char CelciusBuffer[6];
	dtostrf(tempC, 4, 2, CelciusBuffer);
	printf("T(C)=%s\n\r", CelciusBuffer);
			
	char FahrenheitBuffer[6];
	dtostrf(tempF, 4, 2, FahrenheitBuffer);
	printf("T(F)=%s\n\r", FahrenheitBuffer);
	
}

void print_voltage(float avg_Voltage)
{
	
	char VoltageBuffer[6];
	dtostrf(avg_Voltage, 4, 2, VoltageBuffer);
	printf("Voltage=%s\n\r", VoltageBuffer);
}

void buttonSM(void){
	static btn_state_t btn_state1 = RELEASED;
	switch(btn_state1){
		case RELEASED:
		if(BTN1) btn_state1 = MAYBE_PUSHED;
		break;
		case MAYBE_PUSHED:
		if(BTN1){
			btn_state1 = PUSHED;
			btnPushed = true;
		}
		else btn_state1 = RELEASED;
		break;
		case PUSHED:
		if(!BTN1) btn_state1 = MAYBE_RELEASED;
		break;
		case MAYBE_RELEASED:
		if(BTN1) btn_state1 = PUSHED;
		else btn_state1 = RELEASED;
		break;
	}
}