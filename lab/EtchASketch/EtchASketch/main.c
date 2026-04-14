/*
 * EtchASketch.c
 *
 * Created: 4/4/2026 11:24:20 AM
 * Author : samou
 */ 

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

#include "uart.h"
#include "I2C.h"
#include "SH1106.h"

#define ADC_MAX             4095UL
#define SCREEN_X_MAX        (SH1106_WIDTH - 1)    // 127
#define SCREEN_Y_MAX        (SH1106_HEIGHT - 1)   // 63

#define SAMPLE_PERIOD_MS    5
#define REFRESH_PERIOD_MS   25

/* Buttons */
#define BTN_CLEAR_bm        PIN2_bm   // PB2
#define BTN_MODE_bm         PIN3_bm   // PB3

/* FUNCTION PROTOTYPES */
void init_clock(void);
void init_ADC(void);
uint16_t ADC_Read_Channel(uint8_t muxpos);
uint8_t map_adc_to_x(uint16_t adc_val);
uint8_t map_adc_to_y(uint16_t adc_val);
void InitTCA0(void);

static int abs_int(int v);
void draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);

/* GLOBAL VARIABLES */
volatile uint16_t sample_timer = SAMPLE_PERIOD_MS;
volatile uint16_t refresh_timer = REFRESH_PERIOD_MS;

volatile uint8_t cursor_x = 0;
volatile uint8_t cursor_y = 0;

/* Draw mode state machine */
typedef enum
{
    DRAW_SOLID = 0,
    DRAW_DOTTED,
    DRAW_OFF
} draw_mode_t;

int main(void)
{
    init_clock();
    uart_init(3, 9600, NULL);

    init_ADC();
    TWI_Host_Initialize();
    SH1106_Init();

    /* Buttons on PORTB, active low with pull-ups */
    PORTB.DIRCLR = BTN_CLEAR_bm | BTN_MODE_bm;
    PORTB.PIN2CTRL = PORT_PULLUPEN_bm;
    PORTB.PIN3CTRL = PORT_PULLUPEN_bm;

    InitTCA0();
    sei();

    uint8_t x_new, y_new;
    uint8_t prev_x = 0, prev_y = 0;
    uint8_t first_point = 1;

    draw_mode_t draw_mode = DRAW_SOLID;

    SH1106_ClearBuffer();
    SH1106_SetPixel(cursor_x, cursor_y);
    SH1106_UpdateScreen();

    while (1)
    {
        if (sample_timer == 0)
        {
            uint16_t x_adc, y_adc;

            cli();
            sample_timer = SAMPLE_PERIOD_MS;
            sei();

            x_adc = ADC_Read_Channel(ADC_MUXPOS_AIN4_gc);   // PD4
            y_adc = ADC_Read_Channel(ADC_MUXPOS_AIN5_gc);   // PD5

            x_new = map_adc_to_x(x_adc);
            y_new = map_adc_to_y(y_adc);

            if (first_point)
            {
                cursor_x = x_new;
                cursor_y = y_new;
                prev_x = x_new;
                prev_y = y_new;

                /* show initial point only if not in OFF mode */
                if (draw_mode != DRAW_OFF)
                {
                    SH1106_SetPixel(cursor_x, cursor_y);
                }

                first_point = 0;
            }
            else
            {
                prev_x = cursor_x;
                prev_y = cursor_y;

                cursor_x = x_new;
                cursor_y = y_new;

                switch (draw_mode)
                {
                    case DRAW_SOLID:
                        draw_line(prev_x, prev_y, cursor_x, cursor_y);
                        break;

                    case DRAW_DOTTED:
                        SH1106_SetPixel(cursor_x, cursor_y);
                        break;

                    case DRAW_OFF:
                        /* move silently */
                        break;

                    default:
                        break;
                }
            }
        }

        if (refresh_timer == 0)
        {
            cli();
            refresh_timer = REFRESH_PERIOD_MS;
            sei();

            SH1106_UpdateScreen();
        }

        /* Clear button: PB2 */
        if (!(PORTB.IN & BTN_CLEAR_bm))
        {
            SH1106_ClearBuffer();

            /* redraw current cursor only if visible mode */
            if (draw_mode != DRAW_OFF)
            {
                SH1106_SetPixel(cursor_x, cursor_y);
            }

            SH1106_UpdateScreen();

            while (!(PORTB.IN & BTN_CLEAR_bm));
            _delay_ms(20);
        }

        /* Mode button: PB3
           SOLID -> DOTTED -> OFF -> SOLID ...
        */
        if (!(PORTB.IN & BTN_MODE_bm))
        {
            draw_mode = (draw_mode_t)((draw_mode + 1) % 3);

            /* Reset path anchor so switching back to solid
               does not draw a long connecting line. */
            prev_x = cursor_x;
            prev_y = cursor_y;

            /* Optional UART feedback */
            if (draw_mode == DRAW_SOLID)
                printf("Mode: SOLID\r\n");
            else if (draw_mode == DRAW_DOTTED)
                printf("Mode: DOTTED\r\n");
            else
                printf("Mode: OFF\r\n");

            while (!(PORTB.IN & BTN_MODE_bm));
            _delay_ms(20);
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
    return (uint8_t)(((uint32_t)adc_val * SCREEN_X_MAX) / ADC_MAX);
}

uint8_t map_adc_to_y(uint16_t adc_val)
{
    return (uint8_t)(((uint32_t)adc_val * SCREEN_Y_MAX) / ADC_MAX);
}

/* LINE DRAWING */

static int abs_int(int v)
{
    return (v < 0) ? -v : v;
}

void draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
    int dx = abs_int((int)x1 - (int)x0);
    int sx = (x0 < x1) ? 1 : -1;
    int dy = -abs_int((int)y1 - (int)y0);
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;

    while (1)
    {
        SH1106_SetPixel(x0, y0);

        if (x0 == x1 && y0 == y1)
            break;

        int e2 = 2 * err;

        if (e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }

        if (e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
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