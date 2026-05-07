/*
 * EtchASketch.c
 *
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
#include <stdlib.h>
#include <stdbool.h>

#include "uart.h"
#include "I2C.h"
#include "SH1106.h"
#include "GFX.h"

#define ADC_MAX             4095UL

#define SAMPLE_PERIOD_MS    5
#define REFRESH_PERIOD_MS   25
#define CURSOR_BLINK_TICKS  5

#define DEBOUNCE_TIME		10

/* Buttons */
#define BTN_CLEAR_bm		PIN2_bm   // PB2
#define BTN_MODE_bm			PIN4_bm // PB4
#define BTN_NODRAW_bm		PIN5_bm //PB5

#define BTN_CLEAR (!(PORTB.IN & PIN2_bm))
#define BTN_MODE (!(PORTB.IN & PIN4_bm))
#define BTN_NODRAW (!(PORTB.IN & PIN5_bm))

/* FUNCTION PROTOTYPES */
void init_clock(void);
void init_ADC(void);
uint16_t ADC_Read_Channel(uint8_t muxpos);
uint8_t map_adc_to_x(uint16_t adc_val);
uint8_t map_adc_to_y(uint16_t adc_val);
void InitTCA0(void);
void buttonSM1(void);
void buttonSM2(void);
void buttonSM3(void);

/* TYPEDEFS */
typedef enum {RELEASED, MAYBE_PUSHED, PUSHED, MAYBE_RELEASED
} btn_state_t;

typedef enum {
	MODE_DRAW,
	MODE_LIFE,
	MODE_NODRAW
} app_mode_t;



/* GLOBAL VARIABLES */
volatile uint16_t sample_timer = SAMPLE_PERIOD_MS;
volatile uint16_t refresh_timer = REFRESH_PERIOD_MS;
volatile uint16_t debounce_timer1 = DEBOUNCE_TIME;
volatile uint16_t debounce_timer2 = DEBOUNCE_TIME;
volatile uint16_t debounce_timer3 = DEBOUNCE_TIME;

volatile bool btn1Pushed = false;
volatile bool btn2Pushed = false;
volatile bool btn3Pushed = false;

volatile uint8_t cursor_x = 0;
volatile uint8_t cursor_y = 0;

volatile app_mode_t mode = MODE_DRAW;

int main(void)
{
    init_clock();
    uart_init(3, 9600, NULL);

    init_ADC();
    I2C_Init();
    SH1106_Init();

    /* Buttons on PORTB, active low with pull-ups */
    PORTB.DIRCLR = BTN_CLEAR_bm;
	PORTB.DIRCLR = BTN_MODE_bm;
	PORTB.DIRCLR = BTN_NODRAW_bm;
    PORTB.PIN2CTRL = PORT_PULLUPEN_bm;
	PORTB.PIN4CTRL = PORT_PULLUPEN_bm;
	PORTB.PIN5CTRL = PORT_PULLUPEN_bm;

    InitTCA0();
    sei();

    uint8_t x_new, y_new;
	uint16_t x_adc, y_adc;
	
	x_adc = ADC_Read_Channel(ADC_MUXPOS_AIN4_gc);   // PD4
	y_adc = ADC_Read_Channel(ADC_MUXPOS_AIN5_gc);   // PD5

	cursor_x = map_adc_to_x(x_adc);
	cursor_y = map_adc_to_y(y_adc);

    SH1106_ClearBuffer();
    SH1106_UpdateScreen();
	
	SH1106_ClearBuffer();

	draw_string(0, 0, "ETCH A SKETCH");
	draw_string(0, 10, "ECE3411 SPRING 2026");
	draw_string(0, 20, "SAMUEL DINERMAN");

	SH1106_UpdateScreen();
	
	_delay_ms(2000);
	
	SH1106_ClearBuffer();

	draw_string(0, 0, "CONTROLS");
	draw_string(0, 20, "Knobs: Move Stylus");
	draw_string(0, 30, "PB2: Clear Display");
	draw_string(0, 40, "PB4: Conway GOL");
	draw_string(0, 50, "PB5: Lift Stylus");

	SH1106_UpdateScreen();

    while (1)
    {
        /* Sample pots and move cursor */
		if (sample_timer == 0)
		{
			cli();
			sample_timer = SAMPLE_PERIOD_MS;
			sei();

			if (mode == MODE_DRAW)
			{
				gfx_cursor_hide();

				x_adc = ADC_Read_Channel(ADC_MUXPOS_AIN4_gc);
				y_adc = ADC_Read_Channel(ADC_MUXPOS_AIN5_gc);

				x_new = map_adc_to_x(x_adc);
				y_new = map_adc_to_y(y_adc);

				draw_line(cursor_x, cursor_y, x_new, y_new);

				cursor_x = x_new;
				cursor_y = y_new;
			}
			else if (mode == MODE_LIFE)
			{
				gfx_cursor_hide();
				SH1106_UpdateLife();
			}
			else if (mode == MODE_NODRAW)
			{
				x_adc = ADC_Read_Channel(ADC_MUXPOS_AIN4_gc);
				y_adc = ADC_Read_Channel(ADC_MUXPOS_AIN5_gc);

				x_new = map_adc_to_x(x_adc);
				y_new = map_adc_to_y(y_adc);

				cursor_x = x_new;
				cursor_y = y_new;

				gfx_cursor_blink(cursor_x, cursor_y, CURSOR_BLINK_TICKS);
			}
		}

        /* Refresh display */
        if (refresh_timer == 0)
        {
            cli();
            refresh_timer = REFRESH_PERIOD_MS;
            sei();

            SH1106_UpdateScreen();
        }
		
        /* Clear button: PB2 */
		if(debounce_timer1 == 0)
		{
			cli();
			debounce_timer1 = DEBOUNCE_TIME;
			sei();
			buttonSM1();
		}
		
		if(btn1Pushed)
		{
			cli();
			btn1Pushed = false;
			sei();
			gfx_cursor_hide();
			SH1106_ClearBuffer();
			SH1106_SetPixel(cursor_x, cursor_y);
			SH1106_UpdateScreen();
		}
		
		/* Mode button: PB4 */
		if(debounce_timer2 == 0)
		{
			cli();
			debounce_timer2 = DEBOUNCE_TIME;
			sei();
			buttonSM2();			
		}
		if(btn2Pushed)
		{
			cli();
			btn2Pushed = false;
			sei();

			gfx_cursor_hide();

			if (mode == MODE_DRAW)
			{
				mode = MODE_LIFE;
			}
			else
			{
				mode = MODE_DRAW;
			}
		}
		
		/* NO_DRAW BUTTON */
		if(debounce_timer3 == 0)
		{
			cli();
			debounce_timer3 = DEBOUNCE_TIME;
			sei();
			buttonSM3();
		}
		if(btn3Pushed)
		{
			cli();
			btn3Pushed = false;
			sei();
			
			gfx_cursor_hide();

			if(mode == MODE_NODRAW)
			{
				mode = MODE_DRAW;
			}
			else if(mode == MODE_DRAW)
			{
				mode = MODE_NODRAW;
			}
			else
			{
				mode = mode;
			}
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
		
	if (debounce_timer1 > 0)
		debounce_timer1--;
		
	if (debounce_timer2 > 0)
		debounce_timer2--;
		
	if (debounce_timer3 > 0)
		debounce_timer3--;

    TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
}

/* Buttons */
void buttonSM1(void){
	static btn_state_t btn_state1 = RELEASED;
	switch(btn_state1){
		case RELEASED:
		if(BTN_CLEAR) btn_state1 = MAYBE_PUSHED;
		break;
		case MAYBE_PUSHED:
		if(BTN_CLEAR){
			btn_state1 = PUSHED;
			btn1Pushed = true;
		}
		else btn_state1 = RELEASED;
		break;
		case PUSHED:
		if(!BTN_CLEAR) btn_state1 = MAYBE_RELEASED;
		break;
		case MAYBE_RELEASED:
		if(BTN_CLEAR) btn_state1 = PUSHED;
		else btn_state1 = RELEASED;
		break;
	}
	
}

void buttonSM2(){
	static btn_state_t btn_state2 = RELEASED;
	switch(btn_state2){
		case RELEASED:
		if(BTN_MODE) btn_state2 = MAYBE_PUSHED;
		break;
		case MAYBE_PUSHED:
		if(BTN_MODE){
			btn_state2 = PUSHED;
			btn2Pushed = true;
		}
		else btn_state2 = RELEASED;
		break;
		case PUSHED:
		if(!BTN_MODE) btn_state2 = MAYBE_RELEASED;
		break;
		case MAYBE_RELEASED:
		if(BTN_MODE) btn_state2 = PUSHED;
		else btn_state2 = RELEASED;
		break;
	}
	
}

void buttonSM3(){
	static btn_state_t btn_state3 = RELEASED;
	switch(btn_state3){
		case RELEASED:
		if(BTN_NODRAW) btn_state3 = MAYBE_PUSHED;
		break;
		case MAYBE_PUSHED:
		if(BTN_NODRAW){
			btn_state3 = PUSHED;
			btn3Pushed = true;
		}
		else btn_state3 = RELEASED;
		break;
		case PUSHED:
		if(!BTN_NODRAW) btn_state3 = MAYBE_RELEASED;
		break;
		case MAYBE_RELEASED:
		if(BTN_NODRAW) btn_state3 = PUSHED;
		else btn_state3 = RELEASED;
		break;
	}
	
}