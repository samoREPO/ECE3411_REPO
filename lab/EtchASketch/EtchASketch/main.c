/*
 * EtchASketch.c
 *
 * Created: 4/4/2026 11:24:20 AM
 * Author : samou
 */ 

/*
 * AVR128DB48 + SH1106
 * Pot on AIN4 -> X
 * Pot on AIN5 -> Y
 */

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdint.h>

#include "uart.h"
#include "I2C.h"
#include "SH1106.h"

#define ADC_MAX         4095UL
#define SCREEN_X_MAX    (SH1106_WIDTH - 1)    // 127
#define SCREEN_Y_MAX    (SH1106_HEIGHT - 1)   // 63

// Timer ISR fires every 1 ms with your current setup.
// Use separate software timers for sampling and display refresh.
#define SAMPLE_PERIOD_MS    10
#define REFRESH_PERIOD_MS   40

/* FUNCTION PROTOTYPES */
void init_clock(void);
void init_ADC(void);
uint16_t ADC_Read_Channel(uint8_t muxpos);
uint8_t map_adc_to_x(uint16_t adc_val);
uint8_t map_adc_to_y(uint16_t adc_val);
void InitTCA0(void);

/* GLOBAL VARIABLES */
volatile uint16_t sample_timer = SAMPLE_PERIOD_MS;
volatile uint16_t refresh_timer = REFRESH_PERIOD_MS;

volatile uint8_t cursor_x = 0;
volatile uint8_t cursor_y = 0;

int main(void)
{
    init_clock();
    uart_init(3, 9600, NULL);

    init_ADC();
    TWI_Host_Initialize();
    SH1106_Init();

    InitTCA0();
    sei();

    SH1106_ClearBuffer();
    SH1106_SetPixel(cursor_x, cursor_y);
    SH1106_UpdateScreen();

    while (1)
    {
        if (sample_timer == 0)
        {
            uint16_t x_adc, y_adc;
            uint8_t x_new, y_new;

            cli();
            sample_timer = SAMPLE_PERIOD_MS;
            sei();

            x_adc = ADC_Read_Channel(ADC_MUXPOS_AIN4_gc);   // PD4
            y_adc = ADC_Read_Channel(ADC_MUXPOS_AIN5_gc);   // PD5

            x_new = map_adc_to_x(x_adc);
            y_new = map_adc_to_y(y_adc);

            cursor_x = x_new;
            cursor_y = y_new;

            // Etch-a-sketch behavior: keep drawing, never erase
            SH1106_SetPixel(cursor_x, cursor_y);
        }

        if (refresh_timer == 0)
        {
            cli();
            refresh_timer = REFRESH_PERIOD_MS;
            sei();

            SH1106_UpdateScreen();
        }
		if (!(PORTB.IN & PIN2_bm))   // active-low button example
		{
			SH1106_ClearBuffer();
			SH1106_SetPixel(cursor_x, cursor_y);
		}
    }
}

/* ADC FUNCTIONS */

void init_ADC(void)
{
    ADC0.MUXPOS = ADC_MUXPOS_AIN4_gc;
    ADC0.CTRLC  = ADC_PRESC_DIV16_gc;          // CLK_ADC = 1 MHz
    ADC0.CTRLD  = ADC_INITDLY_DLY16_gc;        // 16 us startup delay
    VREF.ADC0REF = VREF_REFSEL_VDD_gc;         // VDD reference
    ADC0.CTRLA |= ADC_ENABLE_bm;               // enable ADC
    ADC0.COMMAND = ADC_STCONV_bm;              // dummy first conversion

    while (ADC0.COMMAND & ADC_STCONV_bm);
}

uint16_t ADC_Read_Channel(uint8_t muxpos)
{
    ADC0.MUXPOS = muxpos;
    ADC0.COMMAND = ADC_STCONV_bm;
    loop_until_bit_is_clear(ADC0.COMMAND, ADC_STCONV_bp);
    return ADC0.RES;
}

uint8_t map_adc_to_x(uint16_t adc_val)
{
	return (uint8_t)(((uint32_t)adc_val * (SH1106_WIDTH - 1)) / ADC_MAX);
}

uint8_t map_adc_to_y(uint16_t adc_val)
{
	return (uint8_t)(((uint32_t)adc_val * (SH1106_HEIGHT - 1)) / ADC_MAX);
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

/* TCA */

void InitTCA0(void)
{
    TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc;
    TCA0.SINGLE.PER = 999;                             // 1000 ticks
    TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;          // overflow interrupt
    TCA0.SINGLE.CTRLA |= TCA_SINGLE_CLKSEL_DIV16_gc;  // 1 us/tick at 16 MHz
    TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;
}

ISR(TCA0_OVF_vect)
{
    if (sample_timer > 0)
        sample_timer--;

    if (refresh_timer > 0)
        refresh_timer--;

    TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
}