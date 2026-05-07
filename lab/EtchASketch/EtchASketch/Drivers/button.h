/*
 * button.h
 *
 * Created: 4/20/2026 2:47:12 PM
 *  Author: samou
 */ 

#ifndef BUTTON_H_
#define BUTTON_H_

#include <stdint.h>
#include <avr/io.h>

typedef struct
{
	PORT_t *port;
	uint8_t pin_bm;
	uint8_t last_raw;
	uint8_t stable_state;
	uint8_t pressed_event;
	uint16_t debounce_count;
} Button_t;

void Button_Init(Button_t *button, PORT_t *port, uint8_t pin_bm);
void Button_Update(Button_t *button);

uint8_t Button_IsPressed(Button_t *button);
uint8_t Button_WasPressed(Button_t *button);

#endif /* BUTTON_H_ */