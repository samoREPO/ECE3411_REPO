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

#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/interrupt.h>

/*** FUNCTION PROTOTYPES ***/
void SPI_Host_Init(void);
uint8_t SPI_Host_Transceiver(uint8_t cData);
void MPC4131_write(uint16_t data);
void SPI_Client_Init(void);
uint8_t SPI_Client_Receive(void);
void init_ADC(void);
void init_AC(void);
void InitTCA0(void);


volatile uint16_t one_sec_counter = 1000;

int main(void)
{
    init_ADC();
	init_AC();
	SPI_Host_Init();
	InitTCA0();
	sei();
	
    while (1) 
    {
		if(one_sec_counter == 0)
		{
			cli();
			one_sec_counter = 1000;
			sei();
		}
    }
}

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

void MPC4131_write(uint16_t data)
{
	uint16_t frame = (data & 0x03FF);
	uint8_t frameH = ((frame & 0xFF00) >> 8);
	uint8_t frameL = frame & 0x00FF;
	
	SPI0.CTRLB &= !SPI_SSD_bm;
	SPI_Host_Transceiver(frameH);
	SPI_Host_Transceiver(frameL);
	SPI0.CTRLB |= SPI_SSD_bm;
	
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

// -- Analog Comparator -- //
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