/*
 * Lab12_SPI_DigitalPot.c
 *
 * Created: 3/28/2026 3:08:58 PM
 * Author : samou
 */ 

#define F_CPU 16000000UL
#define SPI_PORT PORTA
#define SPI_MOSI PIN4_bm
#define SPI_MISO PIN5_bm
#define SPI_SCK PIN6_bm
#define SPI_SS PIN7_bm
#define MPC4131_CMD_WRITE 0x00
#define ADC_MAX 4095
#define CMPSTATE_bm 0x10

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "lcd_lib.h"
#include "I2C_lib.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*** FUNCTION PROTOTYPES ***/
void SPI_Host_Init(void);
uint8_t SPI_Host_Transceiver(uint8_t cData);
void MPC4131_write(uint16_t data);
void SPI_Client_Init(void);
uint8_t SPI_Client_Receive(void);
void init_ADC(void);
void init_AC(void);
void InitTCA0(void);
void config_MPC4131(void);
void init_clock(void);
void display_voltages(void);
void ADC_POT(void);
void ADC_digitalPOT(void);


volatile uint16_t one_sec_counter = 1000;

int main(void)
{
    // Initializations:
    // -- LCD
    init_clock();
    TWI_Host_Initialize();
    LCDinitialize();
	
	// -- ADC
    init_ADC();
	
	// -- AC
	init_AC();
	
	// -- SPI
	SPI_Host_Init();
	
	// -- TIMER
	InitTCA0();
	
	sei(); //  enable interrupts
	
    while (1) 
    {
		if(one_sec_counter == 0)
		{
			cli();
			one_sec_counter = 1000;
			sei();
			display_voltages();
		}
    }
}

// -- SPI FUNCTIONS -- //
void SPI_Host_Init(void)
{
	// Set MOSI, SCK, and SS output, all others input
	SPI_PORT.DIRSET = SPI_MOSI | SPI_SCK | SPI_SS;
	SPI_PORT.DIRCLR = SPI_MISO;
	/* Enable SPI, Host, set clock rate fck/128 (125 kHz) */
	SPI0.CTRLA = SPI_ENABLE_bm | SPI_MASTER_bm | SPI_PRESC_DIV128_gc;
}

uint8_t SPI_Host_Transceiver(uint8_t cData)
{
	SPI0.DATA = cData; // Start transmission - SS pulled down by hardware
	while(!(SPI0.INTFLAGS & SPI_IF_bm)); // Wait for transmission complete
	return SPI0.DATA; // return received data
}

void SPI_Client_Init(void)
{
	/* Set MISO output, all others input */
	SPI_PORT.DIRSET = SPI_MISO;
	SPI_PORT.DIRCLR = SPI_MOSI | SPI_SCK | SPI_SS;
	/* Enable SPI */
	SPI0.CTRLA = SPI_ENABLE_bm;
}

uint8_t SPI_Client_Receive(void)
{
	/* Wait for reception complete */
	while(!(SPI0.INTFLAGS & SPI_IF_bm));
	/* Return Data Register */
	return SPI0.DATA;
}

// -- ADC FUNCTIONS -- //
void init_ADC(){
	ADC0.MUXPOS = ADC_MUXPOS_AIN8_gc; // Input voltage at AIN8 PORTE0
	ADC0.CTRLC = ADC_PRESC_DIV16_gc; // Set CLK_ADC to 1MHz (valid range from 125kHz to 2MHz)
	ADC0.CTRLD = ADC_INITDLY_DLY16_gc; // Delay 16 CLK_ADC cycles - 16 us
	VREF.ADC0REF = VREF_REFSEL_VDD_gc; // VDD as reference
	ADC0.CTRLA |= ADC_ENABLE_bm; // enables the ADC circuitry and left adjusted and 12-bit (3.3/2^12=0.8mV resolution 
	ADC0.COMMAND = ADC_STCONV_bm; // start first A to D conversion
	 
	while(ADC0.COMMAND & ADC_STCONV_bm);
}

void ADC_POT(void)
{
	ADC0.MUXPOS = ADC_MUXPOS_AIN8_gc; // Input voltage at AIN8 PORTE0
}

void ADC_digitalPOT(void)
{
	ADC0.MUXPOS = ADC_MUXPOS_AIN7_gc; // Input voltage at AIN7 PORTD7;
}

// -- Analog Comparator FUNCTIONS -- //
void init_AC(void)
{
	AC0.CTRLA = AC_ENABLE_bm;
	// compare pins PE0 (0, AINP1) and PD7 (0, AINN2)
	AC0.MUXCTRL = (1 << AC_MUXPOS_gp) | (2 << AC_MUXNEG_gp);
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

// -- MPC4131 FUNCTIONS -- //
void MPC4131_write(uint16_t data)
{
	uint16_t frame = (data & 0x03FF);
	uint8_t frameH = ((frame & 0xFF00) >> 8);
	uint8_t frameL = frame & 0x00FF;
	
	SPI_PORT.OUTCLR = SPI_SS;   // CS low
	SPI_Host_Transceiver(frameH);
	SPI_Host_Transceiver(frameL);
	SPI_PORT.OUTSET = SPI_SS;   // CS high
	
}

void config_MPC4131(void)
{
	uint8_t curr_voltage = 0x00;

	for(int i=6; i>=0; i--)
	{
		uint8_t guess = curr_voltage | (0x01 << i);
		MPC4131_write(guess);
		if (AC0.STATUS & CMPSTATE_bm) // AINP (POT) > AINN (DIGIPOT)
		{
			curr_voltage |= (0x01 << i);
		}
	}
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

// -- LCD FUNCTIONS -- //
void display_voltages(void)
{
	ADC_POT();
	ADC0.COMMAND = ADC_STCONV_bm; // start next A to D conversion
	loop_until_bit_is_clear(ADC0.COMMAND, ADC_STCONV_bp);

	
	uint16_t r = ADC0.RES; // TODO: check if i can do this instead of RESL then RESH
	float Voltage = ((float)r/4096.0f) * 3.3f; // 3.3*(RES/(2^12))
		
	char VoltageBuffer[6];
	dtostrf(Voltage, 4, 2, VoltageBuffer);
	
	char LCDout [20] = "V POT: ";
	strcat(LCDout, VoltageBuffer);
		
	LCDgotoXY(0,0);
	LCDstring(LCDout);
	
	config_MPC4131();
	
	ADC_digitalPOT();
	ADC0.COMMAND = ADC_STCONV_bm; // start next A to D conversion
	loop_until_bit_is_clear(ADC0.COMMAND, ADC_STCONV_bp);

	
	r = ADC0.RES; // TODO: check if i can do this instead of RESL then RESH
	Voltage = ((float)r/4096.0f) * 3.3f; // 3.3*(RES/(2^12))
				
	dtostrf(Voltage, 4, 2, VoltageBuffer);
		
	char LCDout2 [20] = "V DPOT: ";
	strcat(LCDout2, VoltageBuffer);
		
	LCDgotoXY(0,1);
	LCDstring(LCDout2);
}