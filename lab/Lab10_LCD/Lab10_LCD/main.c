/*
 * Lab10_LCD.c
 *
 * Created: 3/8/2026 10:42:39 AM
 * Author : samou
 */ 

#include <avr/io.h>
#include <util/twi.h>
#include "lcd_lib.h"
#include "I2C_lib.h"


int main(void)
{
	// I2C setup 
    TWI_Host_Initialize();
	
	//LCD setup
	LCDinitialize();
	
    while (1) 
    {
    }
}

