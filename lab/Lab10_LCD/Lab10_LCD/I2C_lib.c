/*
 * I2C_lib.c
 *
 * Created: 3/8/2026 11:03:02 AM
 *  Author: samou
 */ 

#include "I2C_lib.h"

void TWI_Host_Initialize(void){
	TWI0.MBAUD = 95; // 16MHz clock; rise time = 10ns
	TWI0.MCTRLA |= TWI_ENABLE_bm; // enable the I2C
	TWI0.MSTATUS |= TWI_BUSSTATE_IDLE_gc; // force the bus state to IDLE
}

void TWI_Stop(void){
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

uint8_t TWI_Receive_Data(void){
	// Wait for Read Interrupt Flag.
	while(!(TWI0.MSTATUS & TWI_RIF_bm));
	
	uint8_t data = TWI0.MDATA;
	
	// Respond with NACK
	TWI0.MCTRLB |= TWI_ACKACT_bm;
	
	return data;
}
