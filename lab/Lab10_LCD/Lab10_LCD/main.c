/*
 * Lab10_LCD.c
 *
 * Created: 3/8/2026 10:42:39 AM
 * Author : samou
 */ 

#include <avr/io.h>
#include "lcd_lib.h"
#include "I2C_lib.h"


int main(void)
{
	// I2C setup 
    TWI_Host_Initialize();
	PORTF.PIN2CTRL |= PORT_PULLUPEN_bm; // Pullup resistor on data bus
	PORTF.PIN3CTRL |= PORT_PULLUPEN_bm; // Pullup resistor on SCLK
	
    while (1) 
    {
    }
}

