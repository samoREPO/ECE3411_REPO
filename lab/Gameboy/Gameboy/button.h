/*
 * button.h
 *
 * Created: 4/20/2026 2:48:37 PM
 *  Author: samou
 */ 

/* Buttons */
#define BTN_CLEAR_bm        PIN2_bm   // PB2
#define BTN_CLEAR (!(PORTB.IN & BTN_CLEAR_bm))

#define BTN_MODE_bm         PIN3_bm   // PB3
#define BTN_MODE (!(PORTB.IN & BTN_MODE_bm))

#ifndef BUTTON_H_
#define BUTTON_H_

void init_buttons(void);

#endif /* BUTTON_H_ */