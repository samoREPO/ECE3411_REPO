/*
 * lab03_3-blinkInputChangesPositionToo.c
 *
 * Created: 2/9/2026 10:57:14 AM
 * Author : samou
 */ 

#define F_CPU 4000000UL // set clk = 4MHz
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#define DIRSET_gm 0xFF
#define OUTTGL_gm 0xFF
#define OUTSET_gm 0xFF
#define OUTCLR_gm 0xFF

#define BUTTON_1 PIN2_bm
#define BUTTON_2 PIN4_bm


void blink(uint8_t freq, uint8_t PINx_bm);
void delay_ms_var(uint16_t ms);

int main(void){

	VPORTD.DIRSET = DIRSET_gm;
	VPORTD.OUTCLR = OUTCLR_gm;
	PORTB.PIN2CTRL |= PORT_PULLUPEN_bm;
	
	uint8_t PINx_bm[] = {PIN0_bm, PIN1_bm, PIN2_bm, PIN3_bm, PIN4_bm, PIN5_bm, PIN6_bm, PIN7_bm};
	uint8_t curr_pin = PINx_bm[3];

	uint8_t curr_freq = 3; // curr_freq: {(1, 'inf')}
	uint8_t curr_dir = 0; // direction: {0: right, 1: left}

	uint8_t button1Handled = 0; //flag that button was pressed
	uint8_t button2Handled = 0; //flag that button was pressed
	uint8_t button1and2Handled = 0; //flag that both buttons were pressed


	while (1)
	{
		// ---- Shift position ---- //
		if(!(PORTB.IN & (BUTTON_1 | BUTTON_2))){
			// check if both buttons are latched
			if(!button1and2Handled){
				if(curr_pin % 7 == 0) curr_dir ^= 1;  // change direction if at either end
				curr_dir ? curr_pin-- : curr_pin++;   // move in current direction
				button1and2Handled = 1; // latch both buttons
			}
		}
		else button1and2Handled = 0; // release latch for both buttons

		// ---- Increment frequency ----//
		if( !(PORTB.IN & BUTTON_1)){
			// check if button1 is latched
			if(!button1Handled){
				curr_freq++;
				button1Handled = 1; // latch button1 so this if doesn't execute repeatedly
			}
		}
		// if the button is released => release the latch on button1
		else button1Handled = 0;

		// ---- Decrement Frequency ---- //
		if( !(PORTB.IN & BUTTON_2)){
			//check if button2 is latched
			if(!button2Handled){
				curr_freq -= (curr_freq > 1) ? 1 : 0;
				button2Handled = 1; // latch button2 so this if doesn't execute repeatedly
			}
		}
		// if the button is released => release the latch on button2
		else button2Handled = 0;
		
		// ---- blink ---- //
		blink(curr_freq, PINx_bm[curr_pin]);			// oscillate pin3 at {curr_freq} Hz for 2s

	}
	return 0;               //should never hit this
}

// Delay function that allows non compile-time constant arguments
void delay_ms_var(uint16_t ms){
	while(ms--){
		_delay_ms(1);
	}
}

void blink(uint8_t freq, uint8_t PINx_bm){
	uint16_t half_period_ms = (1000 / 2) freq;     // recall T = 1/f
		VPORTD.OUTTGL = PINx_bm;
		delay_ms_var(half_period_ms);
		VPORTD.OUTTGL = PINx_bm;
		delay_ms_var(half_period_ms);
}
