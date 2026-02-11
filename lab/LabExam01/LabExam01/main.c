/*
 * LabExam01.c
 *
 * Created: 2/9/2026 1:33:43 PM
 * Author : samou
 */ 

#define F_CPU 4000000UL // set CLK = 4MHz

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#define DIRSET_gm 0xFF
#define OUTTGL_gm 0xFF
#define OUTSET_gm 0xFF
#define OUTCLR_gm 0xFF

#define BUTTON_1 PIN2_bm
#define BUTTON_2 PIN4_bm

void blink(float freq, uint8_t PINx_bm);
void delay_ms_var(uint16_t ms);


int main(void)
{
	PORTD.DIRSET = DIRSET_gm; // task a
    PORTD.OUTCLR = OUTCLR_gm;
	PORTB.PIN2CTRL |= PORT_PULLUPEN_bm;
	
	float curr_freq = 2;
	uint8_t curr_direction = 0; //{curr_direction: (0,left), (1, right) }
	uint8_t curr_byte_value = 0x04;
	uint8_t tmp = 0;
	
	uint8_t state = 0;
	uint8_t state_2 = 0;
	
	uint8_t button1Handled = 0; //flag that button was pressed
	uint8_t button2Handled = 0; //flag that button was pressed
	
    while (1) 
    {
		// task b
		//blink(2, PIN3_bm); 
		
		if(!(PORTB.IN & BUTTON_1)){
			if(!button1Handled){
				state = (state+1) % 3;
				button1Handled = 1;
				switch(state_2){
					case 0:
						state_2 = 1;
						break;
						
					case 1:
						state_2 = 1;
						break;
					
					case 2:
						state_2 = 3;
						break;
					
					case 3:
						state_2 = 1;
						curr_byte_value = 0x00;
						break;
					
				}
			}	
		}
		else button1Handled = 0;
		
		if(!(PORTB.IN & BUTTON_2)){
			if(!button2Handled){
				if(curr_freq == 4) curr_freq = 0.5;
				else curr_freq *= 2;
				button2Handled = 1;
				switch(state_2){
					case 0:
						state_2 = 0;
						break;
								
					case 1:
						state_2 = 2;
						break;
									
					case 2:
						state_2 = 0;
						break;
									
					case 3:
						state_2 = 2;
						break;
										
				}
			}
		}
		else button2Handled = 0;
		
		//task d
		switch(state){
			case 0:
				// task c
				PORTD.OUTCLR = OUTCLR_gm;
				PORTD.OUTSET = curr_byte_value++;
				delay_ms_var(1000 / curr_freq);
				break;
				
			case 1:
				PORTD.OUTCLR = OUTCLR_gm;
				tmp = curr_byte_value & 0x80;
				curr_byte_value <<= 1;
				curr_byte_value |= ((tmp>>7) & 0x01);
				PORTD.OUTSET = curr_byte_value;
				delay_ms_var(1000 / curr_freq);
				break;
			
			case 2:
				PORTD.OUTCLR = OUTCLR_gm;
				if(curr_direction == 0){
					curr_byte_value <<= 1;
					curr_byte_value |= 0x01;
					if(curr_byte_value == 0xFF){
						curr_direction = 1;
					}
				}
				else if(curr_direction == 1){
					curr_byte_value >>= 1;
					curr_byte_value &= 0x7F;
					if(curr_byte_value == 0x00){
						curr_direction = 0;	
					}
				}
				PORTD.OUTSET = curr_byte_value;
				delay_ms_var(1000 / curr_freq);
				break;
			
		}
		
		
    }
}

void delay_ms_var(uint16_t ms){
	while(ms--){
		_delay_ms(1);
	}
}

void blink(float freq, uint8_t PINx_bm){
	uint16_t half_period_ms = (uint16_t)((1000 / 2) / freq);     // recall T = 1/f
	PORTD.OUTTGL = PINx_bm;
	delay_ms_var(half_period_ms);
	PORTD.OUTTGL = PINx_bm;
	delay_ms_var(half_period_ms);
}

