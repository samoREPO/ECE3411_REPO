/*
 * FinalProject.c
 *
 * Created: 4/1/2026 7:38:23 PM
 * Author : samou
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include "I2C.h"
#include "ssh1106.h"

int main(void)
{
	TWI_Host_Initialize(true);
	SH1106_Init();

	while (1) {
		SH1106_Fill();
		_delay_ms(1000);

		SH1106_Clear();
		_delay_ms(1000);
	}
}

