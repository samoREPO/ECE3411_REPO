/*
 * etch_sketch.c
 *
 * Created: 4/25/2026 12:31:55 PM
 *  Author: samou
 */ 


#include "etch_sketch.h"

#include "../Drivers/ADC.h"
#include "../Drivers/button.h"
#include "../Drivers/TCA.h"
#include "../GFX/gfx.h"

#include <avr/io.h>
#include <stdint.h>

#define ADC_MAX_VALUE        4095UL

#define ETCH_X_ADC_CHANNEL   ADC_MUXPOS_AIN4_gc
#define ETCH_Y_ADC_CHANNEL   ADC_MUXPOS_AIN5_gc

#define ETCH_SAMPLE_MS       5
#define ETCH_REFRESH_MS      25

#define ETCH_CLEAR_PORT      PORTB
#define ETCH_CLEAR_PIN       PIN2_bm

#define ETCH_MODE_PORT       PORTB
#define ETCH_MODE_PIN        PIN4_bm

typedef enum
{
	ETCH_DRAW_SOLID = 0,
	ETCH_DRAW_DOTTED,
	ETCH_DRAW_CURSOR_OFF
} EtchDrawMode_t;

static Button_t clear_button;
static Button_t mode_button;

static uint8_t cursor_x = 0;
static uint8_t cursor_y = 0;

static uint8_t previous_x = 0;
static uint8_t previous_y = 0;

static uint8_t first_point = 1;

static uint32_t last_sample_time = 0;
static uint32_t last_refresh_time = 0;

static EtchDrawMode_t draw_mode = ETCH_DRAW_SOLID;

static uint8_t map_adc_to_x(uint16_t adc_value);
static uint8_t map_adc_to_y(uint16_t adc_value);

static void sample_inputs(void);
static void handle_buttons(void);

void EtchSketch_Init(void)
{
	Button_Init(&clear_button, &ETCH_CLEAR_PORT, ETCH_CLEAR_PIN);
	Button_Init(&mode_button, &ETCH_MODE_PORT, ETCH_MODE_PIN);

	GFX_Clear();

	cursor_x = 0;
	cursor_y = 0;
	previous_x = 0;
	previous_y = 0;

	first_point = 1;

	last_sample_time = TCA0_Millis();
	last_refresh_time = TCA0_Millis();

	draw_mode = ETCH_DRAW_SOLID;

	GFX_DrawPixel(cursor_x, cursor_y, GFX_COLOR_ON);
	GFX_Update();
}

void EtchSketch_Update(void)
{
	Button_Update(&clear_button);
	Button_Update(&mode_button);
	handle_buttons();

	if (TCA0_HasElapsed(&last_sample_time, ETCH_SAMPLE_MS))
	{
		sample_inputs();
	}

	if (TCA0_HasElapsed(&last_refresh_time, ETCH_REFRESH_MS))
	{
		GFX_Update();
	}
}

static void handle_buttons(void)
{
	if (Button_WasPressed(&clear_button))
	{
		GFX_Clear();

		first_point = 1;

		if (draw_mode != ETCH_DRAW_CURSOR_OFF)
		{
			GFX_DrawPixel(cursor_x, cursor_y, GFX_COLOR_ON);
		}

		GFX_Update();
	}

	if (Button_WasPressed(&mode_button))
	{
		draw_mode = (EtchDrawMode_t)((draw_mode + 1) % 3);
		first_point = 1;
	}
}

static void sample_inputs(void)
{
	uint16_t x_adc = ADC_ReadChannel(ETCH_X_ADC_CHANNEL);
	uint16_t y_adc = ADC_ReadChannel(ETCH_Y_ADC_CHANNEL);

	uint8_t new_x = map_adc_to_x(x_adc);
	uint8_t new_y = map_adc_to_y(y_adc);

	if (first_point)
	{
		cursor_x = new_x;
		cursor_y = new_y;

		previous_x = cursor_x;
		previous_y = cursor_y;

		if (draw_mode != ETCH_DRAW_CURSOR_OFF)
		{
			GFX_DrawPixel(cursor_x, cursor_y, GFX_COLOR_ON);
		}

		first_point = 0;
		return;
	}

	previous_x = cursor_x;
	previous_y = cursor_y;

	cursor_x = new_x;
	cursor_y = new_y;

	switch (draw_mode)
	{
		case ETCH_DRAW_SOLID:
		GFX_DrawLine(previous_x, previous_y, cursor_x, cursor_y, GFX_COLOR_ON);
		break;

		case ETCH_DRAW_DOTTED:
		GFX_DrawDottedLine(previous_x, previous_y, cursor_x, cursor_y, GFX_COLOR_ON);
		break;

		case ETCH_DRAW_CURSOR_OFF:
		break;

		default:
		GFX_DrawLine(previous_x, previous_y, cursor_x, cursor_y, GFX_COLOR_ON);
		break;
	}
}

static uint8_t map_adc_to_x(uint16_t adc_value)
{
	return (uint8_t)(((uint32_t)adc_value * (GFX_WIDTH - 1)) / ADC_MAX_VALUE);
}

static uint8_t map_adc_to_y(uint16_t adc_value)
{
	return (uint8_t)(((uint32_t)adc_value * (GFX_HEIGHT - 1)) / ADC_MAX_VALUE);
}
