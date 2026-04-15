/*
 * ConwayGOL.c
 *
 * Created: 4/14/2026 1:18:48 PM
 * Author : samou
 */ 

/*
 * Conway's Game of Life
 * AVR128DB48 + SH1106
 *
 * Fast version:
 * - Simulates 64x32 cells
 * - Draws each cell as a 2x2 pixel block
 * - Uses wraparound edges
 * - Random reseed on PB2 press
 */

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "uart.h"
#include "I2C.h"
#include "SH1106.h"

#define LIFE_W 64
#define LIFE_H 32
#define LIFE_STEP_DELAY_MS 90

static uint8_t life_current[LIFE_H][LIFE_W];
static uint8_t life_next[LIFE_H][LIFE_W];

/* simple pseudo-random generator state */
static uint32_t lfsr = 0xACE1u;

/* function prototypes */
void init_clock(void);

static uint8_t rand8(void);
void life_clear(void);
void life_seed_random(void);
uint8_t life_count_neighbors(uint8_t x, uint8_t y);
void life_step(void);
void life_draw_to_oled(void);
void draw_cell_2x2(uint8_t gx, uint8_t gy);

/* ------------------------------------------------------------------------- */

int main(void)
{
    init_clock();
    uart_init(3, 9600, NULL);

    TWI_Host_Initialize();
    SH1106_Init();

    /* PB2 active-low button */
    PORTB.DIRCLR = PIN2_bm;
    PORTB.PIN2CTRL = PORT_PULLUPEN_bm;

    sei();

    life_seed_random();
    life_draw_to_oled();
    SH1106_UpdateScreen();

    printf("Conway's Game of Life start\r\n");

    while (1)
    {
        /* PB2 reseeds the board */
        if (!(PORTB.IN & PIN2_bm))
        {
            life_seed_random();
            life_draw_to_oled();
            SH1106_UpdateScreen();

            while (!(PORTB.IN & PIN2_bm));
            _delay_ms(20);
        }

        life_step();
        life_draw_to_oled();
        SH1106_UpdateScreen();

        _delay_ms(LIFE_STEP_DELAY_MS);
    }
}

/* ------------------------------------------------------------------------- */

static uint8_t rand8(void)
{
    /* 16-bit LFSR style update, returned as 8-bit */
    lfsr = (lfsr >> 1) ^ (-(lfsr & 1u) & 0xB400u);
    return (uint8_t)lfsr;
}

void life_clear(void)
{
    memset(life_current, 0, sizeof(life_current));
    memset(life_next, 0, sizeof(life_next));
}

void life_seed_random(void)
{
    life_clear();

    for (uint8_t y = 0; y < LIFE_H; y++)
    {
        for (uint8_t x = 0; x < LIFE_W; x++)
        {
            /* about 29% alive */
            if (rand8() < 75)
            {
                life_current[y][x] = 1;
            }
        }
    }
}

uint8_t life_count_neighbors(uint8_t x, uint8_t y)
{
    uint8_t count = 0;

    for (int8_t dy = -1; dy <= 1; dy++)
    {
        for (int8_t dx = -1; dx <= 1; dx++)
        {
            if (dx == 0 && dy == 0)
                continue;

            uint8_t nx = (uint8_t)((x + dx + LIFE_W) % LIFE_W);
            uint8_t ny = (uint8_t)((y + dy + LIFE_H) % LIFE_H);

            count += life_current[ny][nx];
        }
    }

    return count;
}

void life_step(void)
{
    for (uint8_t y = 0; y < LIFE_H; y++)
    {
        for (uint8_t x = 0; x < LIFE_W; x++)
        {
            uint8_t alive = life_current[y][x];
            uint8_t n = life_count_neighbors(x, y);

            if (alive)
            {
                life_next[y][x] = (n == 2 || n == 3) ? 1 : 0;
            }
            else
            {
                life_next[y][x] = (n == 3) ? 1 : 0;
            }
        }
    }

    memcpy(life_current, life_next, sizeof(life_current));
    memset(life_next, 0, sizeof(life_next));
}

void draw_cell_2x2(uint8_t gx, uint8_t gy)
{
    uint8_t px = gx * 2;
    uint8_t py = gy * 2;

    SH1106_SetPixel(px,     py);
    SH1106_SetPixel(px + 1, py);
    SH1106_SetPixel(px,     py + 1);
    SH1106_SetPixel(px + 1, py + 1);
}

void life_draw_to_oled(void)
{
    SH1106_ClearBuffer();

    for (uint8_t y = 0; y < LIFE_H; y++)
    {
        for (uint8_t x = 0; x < LIFE_W; x++)
        {
            if (life_current[y][x])
            {
                draw_cell_2x2(x, y);
            }
        }
    }
}

/* ------------------------------------------------------------------------- */

void init_clock(void)
{
    CPU_CCP = CCP_IOREG_gc;
    CLKCTRL.XOSCHFCTRLA = CLKCTRL_FRQRANGE_16M_gc | CLKCTRL_ENABLE_bm;

    CPU_CCP = CCP_IOREG_gc;
    CLKCTRL.MCLKCTRLA = CLKCTRL_CLKSEL_EXTCLK_gc;

    while (!(CLKCTRL.MCLKSTATUS & CLKCTRL_EXTS_bm));
}