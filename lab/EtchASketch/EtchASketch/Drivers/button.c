/*
 * button.c
 *
 * Created: 4/20/2026 2:47:23 PM
 *  Author: samou
 */ 


#include "button.h"

#define BUTTON_DEBOUNCE_TICKS 20

static void configure_button_pin(PORT_t *port, uint8_t pin_bm)
{
	uint8_t pin_ctrl = PORT_PULLUPEN_bm | PORT_ISC_INTDISABLE_gc;

	if (pin_bm & PIN0_bm) port->PIN0CTRL = pin_ctrl;
	if (pin_bm & PIN1_bm) port->PIN1CTRL = pin_ctrl;
	if (pin_bm & PIN2_bm) port->PIN2CTRL = pin_ctrl;
	if (pin_bm & PIN3_bm) port->PIN3CTRL = pin_ctrl;
	if (pin_bm & PIN4_bm) port->PIN4CTRL = pin_ctrl;
	if (pin_bm & PIN5_bm) port->PIN5CTRL = pin_ctrl;
	if (pin_bm & PIN6_bm) port->PIN6CTRL = pin_ctrl;
	if (pin_bm & PIN7_bm) port->PIN7CTRL = pin_ctrl;
}

void Button_Init(Button_t *button, PORT_t *port, uint8_t pin_bm)
{
	button->port = port;
	button->pin_bm = pin_bm;

	button->port->DIRCLR = pin_bm;
	configure_button_pin(button->port, button->pin_bm);

	uint8_t raw_pressed = ((button->port->IN & button->pin_bm) == 0);

	button->last_raw = raw_pressed;
	button->stable_state = raw_pressed;
	button->pressed_event = 0;
	button->debounce_count = 0;
}

void Button_Update(Button_t *button)
{
	// Active-low button: pressed when pin reads 0
	uint8_t raw_pressed = ((button->port->IN & button->pin_bm) == 0);

	if (raw_pressed != button->last_raw)
	{
		button->debounce_count = 0;
		button->last_raw = raw_pressed;
	}
	else
	{
		if (button->debounce_count < BUTTON_DEBOUNCE_TICKS)
		{
			button->debounce_count++;
		}
		else
		{
			if (raw_pressed != button->stable_state)
			{
				button->stable_state = raw_pressed;

				if (button->stable_state)
				{
					button->pressed_event = 1;
				}
			}
		}
	}
}

uint8_t Button_IsPressed(Button_t *button)
{
	return button->stable_state;
}

uint8_t Button_WasPressed(Button_t *button)
{
	if (button->pressed_event)
	{
		button->pressed_event = 0;
		return 1;
	}

	return 0;
}
