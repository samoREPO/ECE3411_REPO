/*
 * lab01_1-blink.c
 *
 * Created: 2/9/2026 10:29:59 AM
 * Author : samou
 */ 

////-----------------------------//
///---------- PREAMBLE ---------//
//-----------------------------//

#define F_CPU 4000000UL  // 4 MHz system clock

#include <avr/io.h>
#include <util/delay.h>

void blink(uint8_t pin_bm);

//---------------------------------------------------//

int main(void)
{
	
////------------------------------------//
///---------- INITIALIZATION ----------//
//------------------------------------//
	PORTB.DIRSET = PIN3_bm;
//-------------------------------------//

////--------------------------//
///---------- LOOP ----------//
//--------------------------//
	while (1)
	{
		blink(PIN3_bm);
	}
}

void blink(uint8_t pin_bm)
{
	PORTB.OUTSET = pin_bm;
	_delay_ms(1000);
	PORTB.OUTCLR = pin_bm;
	_delay_ms(1000);
}

