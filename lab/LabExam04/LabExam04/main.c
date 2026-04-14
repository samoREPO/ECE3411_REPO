/*
 * LabExam04.c
 *
 * Created: 4/1/2026 3:16:11 PM
 * Author : samou
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/twi.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(void)
{

	// -- Initializations
	TWI_Host_Initialize(true); // TODO: check wiring to portA
	LCDinitialize();
	
}

