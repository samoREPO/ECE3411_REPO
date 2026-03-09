/*
 * Lab09_I2C.c
 *
 * Created: 3/4/2026 12:19:42 PM
 * Author : samou
 */ 

#define F_CPU 16000000UL
#define TEMP_TIME 500
#define TC74_ADDR 0b1001000

#include <avr/io.h>
#include <util/twi.h>
#include "uart.h"
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>

// I2C prototypes
void TWI_Host_Initialize();
void TWI_Host_Write(uint8_t Address, uint8_t Data);
uint8_t TWI_Host_Read(uint8_t Address);
void TWI_Stop();
void TWI_Address(uint8_t Address, uint8_t mode);
int TWI_Transmit_Data(uint8_t data);
uint8_t TWI_Receive_Data();

// UART prototypes
void init_clock();
void InitTCA0();

volatile uint16_t readTemp_Counter = TEMP_TIME; 

int main(void)
{

	// UART setup
	init_clock();
	uart_init(3,9600,NULL);
	InitTCA0();
	sei();
	
	TWI_Host_Initialize();
	// I2C setup
	PORTA.PIN2CTRL |= PORT_PULLUPEN_bm;
	PORTA.PIN3CTRL |= PORT_PULLUPEN_bm;
    while (1) 
    {
		if(readTemp_Counter == 0){
			cli();
			readTemp_Counter = TEMP_TIME;
			sei();
			TWI_Host_Write(TC74_ADDR, 0);
			uint8_t temperature = TWI_Host_Read(TC74_ADDR);
			printf("Temperature: %d\n", (int)temperature); 	
		}
    }
}

// ------------------ I2C Functions --------------------//
void TWI_Host_Initialize(){
	TWI0.MBAUD = 95; // 16MHz clock; rise time = 10ns
	TWI0.MCTRLA |= TWI_ENABLE_bm; // enable the I2C
	TWI0.MSTATUS |= TWI_BUSSTATE_IDLE_gc; // force the bus state to IDLE
}

void TWI_Host_Write(uint8_t Address, uint8_t Data){
	TWI_Address(Address, TW_WRITE);
	TWI_Transmit_Data(Data);
	TWI_Stop();
}

uint8_t TWI_Host_Read(uint8_t Address){
	TWI_Address(Address, TW_READ);
	uint8_t data = TWI_Receive_Data();
	TWI_Stop();
	return data;
}

void TWI_Stop(){
	TWI0.MCTRLB |= TWI_MCMD_STOP_gc;
}

void TWI_Address(uint8_t Address, uint8_t mode){
	while(1){
		// set addr & R/W bit, starts transfer
		TWI0.MADDR = (Address << 1) | (mode);
		
		// Wait for Read/Write Interrupt Flag
		uint8_t flag = (mode == TW_WRITE) ? TWI_WIF_bp : TWI_RIF_bp;
		loop_until_bit_is_set(TWI0.MSTATUS, flag);
		
		// If the client didn't ack, stop the transaction
		if (TWI0.MSTATUS & TWI_RXACK_bm){
			TWI_Stop();
		}
		
		// If no bus or arbitration error, all good. otherwise try it again
		if(!(TWI0.MSTATUS &(TWI_ARBLOST_bm | TWI_BUSERR_bm))) {break;}
	}
}

int TWI_Transmit_Data(uint8_t data){
	// start data transfer by wiring to MDATA
	TWI0.MDATA = data;
	
	// Wait for Write Interrupt Flag.
	while(!(TWI0.MSTATUS & TWI_WIF_bm));
	
	// if bus or arbitration error, return error
	return ((TWI0.MSTATUS & (TWI_ARBLOST_bm | TWI_BUSERR_bm)) ? -1 : 0);
}

uint8_t TWI_Receive_Data(){
	// Wait for Read Interrupt Flag.
	while(!(TWI0.MSTATUS & TWI_RIF_bm));
	
	uint8_t data = TWI0.MDATA;
	
	// Respond with NACK
	TWI0.MCTRLB |= TWI_ACKACT_bm;
	
	return data;
}


//----------------------- UART Functions ------------------------//
void init_clock() {

	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.XOSCHFCTRLA =  CLKCTRL_FRQRANGE_16M_gc | CLKCTRL_ENABLE_bm;
	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.MCLKCTRLA = CLKCTRL_CLKSEL_EXTCLK_gc;

	while(!(CLKCTRL.MCLKSTATUS & CLKCTRL_EXTS_bm));

}

void InitTCA0(){
	TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc; // Normal mode
	TCA0.SINGLE.PER = 999; // Set number of ticks per period
	
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm; // Enable TCA0 Overflow ISR
	
	TCA0.SINGLE.CTRLA |= TCA_SINGLE_CLKSEL_DIV16_gc; // Set Prescalar to 16. each tick is 1us.
	TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm; // enable timer
}

ISR(TCA0_OVF_vect){
	if(readTemp_Counter>0){readTemp_Counter--;}
	TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
}