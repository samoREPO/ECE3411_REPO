/*
 * Lab06_2-blink_buttons_and_Timers.c
 *
 * Created: 2/14/2026 3:11:08 PM
 * Author : samou
 */ 

#include <avr/io.h>
#include <stdbool.h>

#define DEBOUNCE_TIME 10
#define BTN1 (!(PORTB.IN & PIN2_bm))
#define BTN2 (!(PORTB.IN & PIN3_bm))

typedef enum {RELEASED, MAYBE_PUSHED, PUSHED, MAYBE_RELEASED
	} btn_state_t;

volatile int debounce_timerCount = DEBOUNCE_TIME;
volatile bool btn1Pushed = false;
volatile bool btn2Pushed = false;


// half_period_ms = 1000 / 2 / freq

void init_clock();
void buttonSM();
void handle_btn1_push(); // TODO implement this function body
void handle_btn2_push(); // TODO implement this function body

int main(void)
{
    
    while (1) 
    {
		if(btn1Pushed){
			btn1Pushed = false;
			handle_btn1_push();
		}
		if(btn2Pushed){
			btn2Pushed = false;
			handle_btn2_push();	
		}
    }
}

void init_clock() {

	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.XOSCHFCTRLA =  CLKCTRL_FRQRANGE_16M_gc | CLKCTRL_ENABLE_bm;
	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.MCLKCTRLA = CLKCTRL_CLKSEL_EXTCLK_gc;

	while(!(CLKCTRL.MCLKSTATUS & CLKCTRL_EXTS_bm));

}

void InitTCA0(){
	TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc; // Normal mode
	TCA0.SINGLE.PER = 249; // Set number of ticks per period
	
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm; // Enable TCA0 Overflow ISR
	
	TCA0.SINGLE.CTRLA |= TCA_SINGLE_CLKSEL_DIV64_gc; // Set Prescalar to 64. each tick is 4us.
	TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm; // enable timer
}

ISR(TCA0_OVF_vect){
	//TODO: finish timer ISR
	
	TCA0.SINGLE.INTFLAGS |= TCA_SINGLE_OVF_bm; // must clear the interrupt
}

void buttonSM1(){
	static btn_state_t btn_state1 = RELEASED;
	switch(btn_state1){
		case RELEASED:
			if(BTN1) btn_state1 = MAYBE_PUSHED;
			break;
		case MAYBE_PUSHED:
			if(BTN1){
				btn_state1 = PUSHED;
				btn1Pushed = true;
			}
			else btn_state1 = RELEASED;
			break;
		case PUSHED:
			if(!BTN1) btn_state1 = MAYBE_RELEASED;
			break;
		case MAYBE_RELEASED:
			if(BTN1) btn_state1 = PUSHED;
			else btn_state1 = RELEASED;
			break;
	}
	
}

void buttonSM2(){
	static btn_state_t btn_state2 = RELEASED;
	switch(btn_state2){
		case RELEASED:
		if(BTN2) btn_state2 = MAYBE_PUSHED;
		break;
		case MAYBE_PUSHED:
		if(BTN2){
			btn_state2 = PUSHED;
			btn2Pushed = true;
		}
		else btn_state2 = RELEASED;
		break;
		case PUSHED:
		if(!BTN2) btn_state2 = MAYBE_RELEASED;
		break;
		case MAYBE_RELEASED:
		if(BTN2) btn_state2 = PUSHED;
		else btn_state2 = RELEASED;
		break;
	}
	
}